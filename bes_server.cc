#include <csignal>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"
#include "absl/synchronization/mutex.h"
#include "google/bytestream/bytestream.grpc.pb.h"
#include "google/devtools/build/v1/publish_build_event.grpc.pb.h"
#include "google/protobuf/empty.pb.h"
#include "google/watcher/v1/watch.grpc.pb.h"
#include "grpc++/grpc++.h"
#include "re2/re2.h"
#include "src/main/java/com/google/devtools/build/lib/buildeventstream/proto/build_event_stream.pb.h"

using OuterBuildEvent = google::devtools::build::v1::BuildEvent;
using build_event_stream::ActionExecuted;
using build_event_stream::BuildEvent;
using build_event_stream::BuildStarted;
using build_event_stream::File;
using google::bytestream::ByteStream;
using google::bytestream::ReadRequest;
using google::bytestream::ReadResponse;
using google::devtools::build::v1::OrderedBuildEvent;
using google::devtools::build::v1::PublishBuildEvent;
using google::devtools::build::v1::PublishBuildToolEventStreamRequest;
using google::devtools::build::v1::PublishBuildToolEventStreamResponse;
using google::devtools::build::v1::PublishLifecycleEventRequest;

struct PartialInvocation {
  std::string build_id;
  std::string invocation_id;
  std::string workspace_directory;
  std::vector<std::string> stderr_files;
};

struct PartialBuild {
  std::string build_id;
  bool invocation_started = false;
  bool invocation_finished = false;
  PartialInvocation invocation;
};

struct FinishedInvocation {
  std::string build_id;
  std::string invocation_id;
  std::string workspace_directory;
  std::vector<std::string> stderr_files;
};

struct FinishedBuild {
  std::string build_id;
  FinishedInvocation invocation;
};

class FinishedBuildSink {
 public:
  virtual ~FinishedBuildSink(){};

  virtual void BuildFinished(const FinishedBuild& build) = 0;
};

class NoopFinishedBuildSink : public FinishedBuildSink {
 public:
  void BuildFinished(const FinishedBuild& build) override{};
};

class ErrorFindingFinishedBuildSink : public FinishedBuildSink {
 public:
  ErrorFindingFinishedBuildSink() {
    stop_thread_ = false;
    bytestream_stub_ = ByteStream::NewStub(grpc::CreateChannel(
        "localhost:9092", grpc::InsecureChannelCredentials()));
    thread_ = std::thread([this] { this->Run(); });
  };

  ~ErrorFindingFinishedBuildSink() {
    {
      absl::MutexLock l(&mu_);
      stop_thread_ = true;
    }
    thread_.join();
  };

  void BuildFinished(const FinishedBuild& build) override {
    absl::MutexLock l(&mu_);
    build_queue_.push_back(build);
  }

 private:
  void Run() {
    while (true) {
      mu_.LockWhen(
          absl::Condition(this, &ErrorFindingFinishedBuildSink::IsReady));
      if (stop_thread_) {
        mu_.Unlock();
        return;
      }

      FinishedBuild build = build_queue_.front();
      build_queue_.pop_front();
      mu_.Unlock();

      const FinishedInvocation& invocation = build.invocation;
      if (invocation.workspace_directory !=
          "/Users/endobson/proj/racket/yaspl2") {
        continue;
      }

      std::ofstream output_file;
      output_file.open("/Users/endobson/proj/racket/yaspl2/errors.err",
                       std::ios::binary | std::ios::out | std::ios::trunc);

      for (const auto& stderr_file : invocation.stderr_files) {
        absl::string_view resource = stderr_file;
        if (!absl::ConsumePrefix(&resource, "bytestream://")) {
          continue;
        }
        if (!absl::ConsumePrefix(&resource, "localhost:9092/")) {
          continue;
        }

        grpc::ClientContext context;
        ReadRequest request;
        request.set_resource_name(std::string(resource));
        std::unique_ptr<grpc::ClientReader<ReadResponse>> reader(
            bytestream_stub_->Read(&context, request));
        ReadResponse response;
        std::string stderr_contents;
        while (reader->Read(&response)) {
          stderr_contents += response.data();
        }
        grpc::Status status = reader->Finish();
        if (status.ok()) {
          absl::string_view stderr_view = stderr_contents;
          std::vector<std::string> errors;
          if (IsTypeCheckingErrors(&stderr_view)) {
            errors = GetTypecheckingErrors(stderr_view);
          } else {
            errors = {"ERROR: Unknown errors"};
          }

          for (const auto& error : errors) {
            output_file << error << std::endl;
          }
        } else {
          std::cout << "grpc status: " << status.error_message() << std::endl;
        }
      }
      output_file.close();
    }
  }

  bool IsReady() EXCLUSIVE_LOCKS_REQUIRED(mu_) {
    return stop_thread_ || !this->build_queue_.empty();
  }

  bool IsTypeCheckingErrors(absl::string_view* stderr_contents) {
    static re2::RE2 re("Typechecking failed for module: .*\n\n");
    return re2::RE2::Consume(stderr_contents, re);
  }

  std::vector<std::string> GetTypecheckingErrors(
      absl::string_view stderr_contents) {
    std::vector<std::string> errors;
    static re2::RE2 re(
        "Location: (.*)\\nFunction:.*\\n(?s:(.*?)\\n(?:\\n|\\z))");

    absl::string_view location;
    absl::string_view details;
    while (re2::RE2::Consume(&stderr_contents, re, &location, &details)) {
      std::string single_line_details =
          absl::StrReplaceAll(details, {{"\n", " "}});
      errors.push_back(absl::StrCat(location, ": ", single_line_details));
    }
    if (errors.empty()) {
      errors.push_back("ERROR: Unknown type checking errors");
    }

    return errors;
  }

  absl::Mutex mu_;
  std::deque<FinishedBuild> build_queue_ ABSL_GUARDED_BY(mu_);
  bool stop_thread_ ABSL_GUARDED_BY(mu_);
  std::thread thread_;
  std::unique_ptr<ByteStream::Stub> bytestream_stub_;
};

class PublishBuildEventServiceImpl
    : public google::devtools::build::v1::PublishBuildEvent::Service {
 public:
  PublishBuildEventServiceImpl(FinishedBuildSink* finished_build_sink)
      : finished_build_sink_(finished_build_sink) {}

 private:
  grpc::Status PublishLifecycleEvent(
      grpc::ServerContext* context, const PublishLifecycleEventRequest* request,
      google::protobuf::Empty* reply) override {
    absl::MutexLock l(&mu_);

    const OrderedBuildEvent& ordered = request->build_event();
    const OuterBuildEvent& build_event = ordered.event();

    const std::string& build_id = ordered.stream_id().build_id();
    const std::string& invocation_id = ordered.stream_id().invocation_id();
    switch (build_event.event_case()) {
      case OuterBuildEvent::kBuildEnqueued: {
        PartialBuild partial;
        partial.build_id = build_id;
        partial_builds_.insert({build_id, partial});
        break;
      }
      case OuterBuildEvent::kBuildFinished: {
        const PartialBuild& partial = partial_builds_.at(build_id);

        if (!partial.invocation_started) {
          std::cerr << "Must start partial invocation before finishing build."
                    << std::endl;
          abort();
        }

        if (!partial.invocation_finished) {
          std::cerr << "Must finish partial invocation before finishing build."
                    << std::endl;
          abort();
        }

        FinishedBuild finished;
        finished.build_id = partial.build_id;
        finished.invocation.build_id = partial.invocation.build_id;
        finished.invocation.invocation_id = partial.invocation.invocation_id;
        finished.invocation.workspace_directory =
            partial.invocation.workspace_directory;
        finished.invocation.stderr_files = partial.invocation.stderr_files;

        partial_builds_.erase(build_id);

        std::cout << "Finished build: " << finished.build_id << " "
                  << finished.invocation.invocation_id << std::endl;
        for (const auto& stderr_file : finished.invocation.stderr_files) {
          std::cout << "Stderr available at: " << stderr_file << std::endl;
        }

        finished_build_sink_->BuildFinished(finished);
        break;
      }
      case OuterBuildEvent::kInvocationAttemptStarted: {
        PartialBuild& partial = partial_builds_.at(build_id);
        if (partial.invocation_started) {
          std::cerr << "Support for multiple invocations not yet implemented."
                    << std::endl;
          abort();
        }

        partial.invocation.build_id = build_id;
        partial.invocation.invocation_id = invocation_id;
        partial.invocation_started = true;
        break;
      }

      case OuterBuildEvent::kInvocationAttemptFinished: {
        PartialBuild& partial = partial_builds_.at(build_id);
        if (!partial.invocation_started) {
          std::cerr << "Must start invocation before finishing." << std::endl;
          abort();
        }

        partial.invocation_finished = true;
        break;
      }

      default:
        std::cerr << "Unexpected Lifecycle event:" << std::endl
                  << request->DebugString() << std::endl;
        abort();
        break;
    }

    return grpc::Status::OK;
  }

  grpc::Status PublishBuildToolEventStream(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<PublishBuildToolEventStreamResponse,
                               PublishBuildToolEventStreamRequest>* stream)
      override {
    PublishBuildToolEventStreamRequest request;
    while (stream->Read(&request)) {
      absl::MutexLock l(&mu_);
      const OrderedBuildEvent& ordered = request.ordered_build_event();
      const OuterBuildEvent& outer_build_event = ordered.event();

      switch (outer_build_event.event_case()) {
        case OuterBuildEvent::kComponentStreamFinished: {
          break;
        }
        case OuterBuildEvent::kBazelEvent: {
          BuildEvent build_event;
          if (!outer_build_event.bazel_event().UnpackTo(&build_event)) {
            std::cerr << "Unexpected event:" << std::endl
                      << outer_build_event.DebugString() << std::endl;
            abort();
          }

          switch (build_event.payload_case()) {
            case BuildEvent::kStarted: {
              const BuildStarted& started = build_event.started();
              PartialBuild& partial =
                  partial_builds_.at(ordered.stream_id().build_id());
              partial.invocation.workspace_directory =
                  started.workspace_directory();
              break;
            }
            case BuildEvent::kAction: {
              const ActionExecuted& action = build_event.action();
              if (action.success()) break;

              if (action.has_stderr() &&
                  action.stderr().file_case() == File::kUri) {
                PartialBuild& partial =
                    partial_builds_.at(ordered.stream_id().build_id());
                // TODO check started and that invocation id matches
                partial.invocation.stderr_files.push_back(
                    action.stderr().uri());
              }

              break;
            }
            default:
              break;
          }
          break;
        }
        default:
          std::cerr << "Unexpected tool event:" << std::endl
                    << request.DebugString() << std::endl;
          abort();
          break;
      };

      PublishBuildToolEventStreamResponse response;
      *response.mutable_stream_id() = ordered.stream_id();
      response.set_sequence_number(ordered.sequence_number());
      stream->Write(response);
    }

    std::cout << "EventStream done:" << std::endl;
    return grpc::Status::OK;
  }

  absl::Mutex mu_;
  absl::flat_hash_map<std::string, PartialBuild> partial_builds_
      ABSL_GUARDED_BY(mu_);
  FinishedBuildSink* finished_build_sink_;
};

grpc::Server* server;

void RunServer() {
  grpc::ServerBuilder builder;

  ErrorFindingFinishedBuildSink sink;
  PublishBuildEventServiceImpl bes_service(&sink);
  builder.RegisterService(&bes_service);

  std::string server_address("0.0.0.0:8089");
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  server = builder.BuildAndStart().release();

  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
  std::cout << "Server shutting down." << std::endl;
}

void signal_handler(int sig) {
  if (server) {
    server->Shutdown();
  }
}

int main(int argc, char** argv) {
  std::signal(SIGINT, signal_handler);

  RunServer();

  return 0;
}
