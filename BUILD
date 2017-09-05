exports_files(["grpc.bzl"])

cc_library(
  name = "bes_server_lib",
  srcs = ["bes_server.cc"],
  deps = [
    "@grpc//:grpc++",
    "@googleapis//:build_event_service_grpc_cc",
    ":build_event_stream_cc_proto",
  ]
)

cc_binary(
  name = "bes_server",
  deps = [
    ":bes_server_lib"
  ]
)
  
cc_proto_library(
  name = "build_event_stream_cc_proto",
  deps = ["@io_bazel//src/main/java/com/google/devtools/build/lib/buildeventstream/proto:build_event_stream_proto"],
)
