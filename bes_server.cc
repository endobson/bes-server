#include <iostream>
#include <string>
#include <grpc++/grpc++.h>

#include "google/devtools/build/v1/publish_build_event.grpc.pb.h"
#include "google/protobuf/empty.pb.h"
#include "src/main/java/com/google/devtools/build/lib/buildeventstream/proto/build_event_stream.pb.h"

using google::devtools::build::v1::OrderedBuildEvent;
using google::devtools::build::v1::PublishBuildEvent;
using google::devtools::build::v1::PublishBuildToolEventStreamResponse;
using google::devtools::build::v1::PublishLifecycleEventRequest;

class PublishBuildEventServiceImpl : public google::devtools::build::v1::PublishBuildEvent::Service {
  grpc::Status PublishLifecycleEvent(
      grpc::ServerContext* context,
      const PublishLifecycleEventRequest* request,
      google::protobuf::Empty* reply) override {

    std::cout << "Lifecycle event:" << std::endl << request->DebugString() << std::endl;

    return grpc::Status::OK;
  }

  grpc::Status PublishBuildToolEventStream(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<PublishBuildToolEventStreamResponse, OrderedBuildEvent>* stream) override {
    OrderedBuildEvent event;
    while (stream->Read(&event)) {
      // std::cout << "EventStream event:" << std::endl << event.DebugString() << std::endl;
      //PublishBuildToolEventStreamResponse response;
      //stream->Write(response);
    }

    return grpc::Status::OK;
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:8089");
  PublishBuildEventServiceImpl service;

  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  std::unique_ptr<grpc::Server> server = builder.BuildAndStart();

  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char** argv) {
  build_event_stream::BuildEvent event;
  RunServer();

  return 0;
}
