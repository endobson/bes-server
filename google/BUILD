package(default_visibility = ["//visibility:public"])

load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

proto_library(
    name = "google_devtools_build_v1_build_status_proto",
    srcs = ["devtools/build/v1/build_status.proto"],
    deps = [
        ":google_api_annotations_proto",
        "@com_google_protobuf//:any_proto",
    ],
)

proto_library(
    name = "google_devtools_build_v1_build_events_proto",
    srcs = ["devtools/build/v1/build_events.proto"],
    deps = [
        ":google_api_annotations_proto",
        ":google_devtools_build_v1_build_status_proto",
        ":google_rpc_status_proto",
        "@com_google_protobuf//:any_proto",
        "@com_google_protobuf//:timestamp_proto",
        "@com_google_protobuf//:wrappers_proto",
    ],
)

proto_library(
    name = "google_devtools_build_v1_publish_build_event_proto",
    srcs = ["devtools/build/v1/publish_build_event.proto"],
    deps = [
        ":google_api_annotations_proto",
        ":google_api_auth_proto",
        ":google_devtools_build_v1_build_events_proto",
        "@com_google_protobuf//:any_proto",
        "@com_google_protobuf//:duration_proto",
        "@com_google_protobuf//:empty_proto",
    ],
)

cc_proto_library(
    name = "google_devtools_build_v1_publish_build_event_cc_proto",
    deps = [
        ":google_devtools_build_v1_publish_build_event_proto"
    ],
)

cc_grpc_library(
    name = "google_devtools_build_v1_publish_build_event_cc_grpc",
    srcs = [
        ":google_devtools_build_v1_publish_build_event_proto"
    ],
    deps = [
        ":google_devtools_build_v1_publish_build_event_cc_proto"
    ],
    grpc_only = True,
)

proto_library(
    name = "google_api_annotations_proto",
    srcs = ["api/annotations.proto"],
    deps = [
        ":google_api_http_proto",
        "@com_google_protobuf//:descriptor_proto",
    ],
)

proto_library(
    name = "google_api_http_proto",
    srcs = ["api/http.proto"],
)

proto_library(
    name = "google_rpc_status_proto",
    srcs = ["rpc/status.proto"],
    deps = ["@com_google_protobuf//:any_proto"],
)

proto_library(
    name = "google_api_auth_proto",
    srcs = ["api/auth.proto"],
    deps = [
        ":google_api_annotations_proto",
        "@com_google_protobuf//:descriptor_proto",
    ],
)

proto_library(
    name = "google_watcher_v1_watch_proto",
    srcs = ["watcher/v1/watch.proto"],
    deps = [
        ":google_api_annotations_proto",
        "@com_google_protobuf//:any_proto",
    ],
)

cc_proto_library(
    name = "google_watcher_v1_watch_cc_proto",
    deps = [
        ":google_watcher_v1_watch_proto"
    ],
)

cc_grpc_library(
    name = "google_watcher_v1_watch_cc_grpc",
    srcs = [
        ":google_watcher_v1_watch_proto"
    ],
    deps = [
        ":google_watcher_v1_watch_cc_proto"
    ],
    grpc_only = True,
)
