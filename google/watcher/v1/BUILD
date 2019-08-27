package(default_visibility = ["//visibility:public"])

load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

proto_library(
    name = "watch_proto",
    srcs = ["watch.proto"],
    deps = [
        "//google/api:annotations_proto",
        "@com_google_protobuf//:any_proto",
    ],
)

cc_proto_library(
    name = "watch_cc_proto",
    deps = [
        ":watch_proto"
    ],
)

cc_grpc_library(
    name = "watch_cc_grpc",
    srcs = [
        ":watch_proto"
    ],
    deps = [
        ":watch_cc_proto"
    ],
    grpc_only = True,
)
