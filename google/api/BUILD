package(default_visibility = ["//visibility:public"])

proto_library(
    name = "annotations_proto",
    srcs = ["annotations.proto"],
    deps = [
        ":http_proto",
        "@com_google_protobuf//:descriptor_proto",
    ],
)

proto_library(
    name = "auth_proto",
    srcs = ["auth.proto"],
    deps = [
        ":annotations_proto",
        "@com_google_protobuf//:descriptor_proto",
    ],
)

proto_library(
    name = "http_proto",
    srcs = ["http.proto"],
)
