exports_files(["grpc.bzl"])

load(":dynamic_libs.bzl", "dynamic_libs")
load("@minimal_racket//:racket.bzl", "racket_library", "racket_binary")

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

dynamic_libs(
  name = "grpc_unsecure_dynamic_libs",
  srcs = ["@grpc//:grpc_unsecure"],
)

cc_binary(
  name = "libgrpc_unsecure.so",
  deps = ["@grpc//:grpc_unsecure"],
  linkshared = 1
)

racket_library(
  name = "grpc-lib",
  srcs = ["grpc-lib.rkt"],
)

racket_binary(
  name = "racket-grpc",
  deps = [":grpc-lib"],
  main_module = "grpc-lib.rkt",
)
