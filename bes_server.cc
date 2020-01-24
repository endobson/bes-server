#include <iostream>
#include <string>
#include <csignal>

#include <grpc++/grpc++.h>

#include "absl/container/flat_hash_map.h"
#include "absl/synchronization/mutex.h"
#include "google/devtools/build/v1/publish_build_event.grpc.pb.h"
#include "google/watcher/v1/watch.grpc.pb.h"
#include "google/protobuf/empty.pb.h"
#include "src/main/java/com/google/devtools/build/lib/buildeventstream/proto/build_event_stream.pb.h"

using OuterBuildEvent = google::devtools::build::v1::BuildEvent;
using google::devtools::build::v1::OrderedBuildEvent;
using google::devtools::build::v1::PublishBuildEvent;
using google::devtools::build::v1::PublishBuildToolEventStreamResponse;
using google::devtools::build::v1::PublishBuildToolEventStreamRequest;
using google::devtools::build::v1::PublishLifecycleEventRequest;
using build_event_stream::ActionExecuted;
using build_event_stream::BuildEvent;
using build_event_stream::BuildStarted;
using build_event_stream::File;

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
  virtual ~FinishedBuildSink() {};

  virtual void BuildFinished(const FinishedBuild& build) = 0;
};

class NoopFinishedBuildSink : public FinishedBuildSink {
 public:
  void BuildFinished(const FinishedBuild& build) override {};
};

class PublishBuildEventServiceImpl : public google::devtools::build::v1::PublishBuildEvent::Service {
 public:
  PublishBuildEventServiceImpl(FinishedBuildSink* finished_build_sink) :
    finished_build_sink_(finished_build_sink) {}

 private:
  grpc::Status PublishLifecycleEvent(
      grpc::ServerContext* context,
      const PublishLifecycleEventRequest* request,
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
          std::cerr << "Must start partial invocation before finishing build." << std::endl;
          abort();
        }

        if (!partial.invocation_finished) {
          std::cerr << "Must finish partial invocation before finishing build." << std::endl;
          abort();
        }

        FinishedBuild finished;
        finished.build_id = partial.build_id;
        finished.invocation.build_id = partial.invocation.build_id;
        finished.invocation.invocation_id = partial.invocation.invocation_id;
        finished.invocation.workspace_directory = partial.invocation.workspace_directory;
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
          std::cerr << "Support for multiple invocations not yet implemented." << std::endl;
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
        std::cerr << "Unexpected Lifecycle event:" << std::endl << request->DebugString() << std::endl;
        abort();
        break;
    }

    //std::cout << "Lifecycle event:" << std::endl << request->DebugString() << std::endl;

    return grpc::Status::OK;
  }

  grpc::Status PublishBuildToolEventStream(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<PublishBuildToolEventStreamResponse,
                               PublishBuildToolEventStreamRequest>* stream) override {
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
            std::cerr << "Unexpected event:" << std::endl << outer_build_event.DebugString() << std::endl;
            abort();
          }

          switch (build_event.payload_case()) {
            case BuildEvent::kStarted: {
              const BuildStarted& started = build_event.started();
              PartialBuild& partial = partial_builds_.at(ordered.stream_id().build_id());
              partial.invocation.workspace_directory = started.workspace_directory();
            }
            case BuildEvent::kAction: {
              const ActionExecuted& action = build_event.action();
              if (action.success()) break;

              if (action.has_stderr() && action.stderr().file_case() == File::kUri) {
                PartialBuild& partial = partial_builds_.at(ordered.stream_id().build_id());
                // TODO check started and that invocation id matches
                partial.invocation.stderr_files.push_back(action.stderr().uri());
              }

              break;
            }
            default:
              break;
          }
          break;
        }
        default:
          std::cerr << "Unexpected tool event:" << std::endl << request.DebugString() << std::endl;
          abort();
          break;
      };

      //std::cout << "EventStream event:" << std::endl << event.DebugString() << std::endl;

      PublishBuildToolEventStreamResponse response;
      *response.mutable_stream_id() = ordered.stream_id();
      response.set_sequence_number(ordered.sequence_number());
      stream->Write(response);
    }

    std::cout << "EventStream done:" << std::endl;
    return grpc::Status::OK;
  }

  absl::Mutex mu_;
  absl::flat_hash_map<std::string, PartialBuild> partial_builds_ ABSL_GUARDED_BY(mu_);
  FinishedBuildSink* finished_build_sink_;
};

class WatcherServiceImpl : public google::watcher::v1::Watcher::Service {

};

grpc::Server* server;

void RunServer() {
  grpc::ServerBuilder builder;

  NoopFinishedBuildSink sink;
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
