cc_library(
  name = "bes_server_lib",
  srcs = ["bes_server.cc"],
  deps = [
    "@com_github_grpc_grpc//:grpc++",
    "@com_google_absl//absl/synchronization",
    "@com_google_absl//absl/container:flat_hash_map",
    "//google:google_devtools_build_v1_publish_build_event_cc_grpc",
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
