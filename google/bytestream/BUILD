package(default_visibility = ["//visibility:public"])

load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

proto_library(
    name = "bytestream_proto",
    srcs = ["bytestream.proto"],
    deps = [
        "//google/api:annotations_proto",
    ],
)

cc_proto_library(
    name = "bytestream_cc_proto",
    deps = [
        ":bytestream_proto"
    ],
)

cc_grpc_library(
    name = "bytestream_cc_grpc",
    srcs = [
        ":bytestream_proto"
    ],
    deps = [
        ":bytestream_cc_proto"
    ],
    grpc_only = True,
)
