package(default_visibility = ["//visibility:public"])

load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

proto_library(
    name = "build_events_proto",
    srcs = ["build_events.proto"],
    deps = [
        "//google/api:annotations_proto",
        ":build_status_proto",
        "//google/rpc:status_proto",
        "@com_google_protobuf//:any_proto",
        "@com_google_protobuf//:timestamp_proto",
        "@com_google_protobuf//:wrappers_proto",
    ],
)

proto_library(
    name = "build_status_proto",
    srcs = ["build_status.proto"],
    deps = [
        "//google/api:annotations_proto",
        "@com_google_protobuf//:any_proto",
    ],
)

proto_library(
    name = "publish_build_event_proto",
    srcs = ["publish_build_event.proto"],
    deps = [
        "//google/api:annotations_proto",
        "//google/api:auth_proto",
        ":build_events_proto",
        "@com_google_protobuf//:any_proto",
        "@com_google_protobuf//:duration_proto",
        "@com_google_protobuf//:empty_proto",
    ],
)

cc_proto_library(
    name = "publish_build_event_cc_proto",
    deps = [
        ":publish_build_event_proto"
    ],
)

cc_grpc_library(
    name = "publish_build_event_cc_grpc",
    srcs = [
        ":publish_build_event_proto"
    ],
    deps = [
        ":publish_build_event_cc_proto"
    ],
    grpc_only = True,
)
