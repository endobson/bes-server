workspace(name = "bes_example")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

# Platforms
http_archive(
    name = "platforms",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/platforms/releases/download/0.0.6/platforms-0.0.6.tar.gz",
        "https://github.com/bazelbuild/platforms/releases/download/0.0.6/platforms-0.0.6.tar.gz",
    ],
    sha256 = "5308fc1d8865406a49427ba24a9ab53087f17f5266a7aabbfc28823f3916e1ca",
)

# GRPC
http_archive(
  name = "com_github_grpc_grpc",
  sha256 = "e034992a0b464042021f6d440f2090acc2422c103a322b0844e3921ccea981dc",
  strip_prefix = "grpc-1.56.0",
  urls = ["https://github.com/grpc/grpc/archive/refs/tags/v1.56.0.tar.gz"],
)

# # BoringSSL since grpc uses a spammy version without a sha256
# git_repository(
#     name = "boringssl",
#     commit = "83da28a68f32023fd3b95a8ae94991a07b1f6c62",
#     shallow_since = "1571438383 +0000",
#     remote = "https://boringssl.googlesource.com/boringssl",
# )
# 
# # abseil-cpp
# http_archive(
#   name = "com_google_absl",
#   urls = ["https://github.com/abseil/abseil-cpp/archive/refs/tags/20230125.3.tar.gz"],
#   strip_prefix = "abseil-cpp-20230125.3",
#   sha256 = "5366d7e7fa7ba0d915014d387b66d0d002c03236448e1ba9ef98122c13b35c36",
# )
# 

# re2 (2020-01-01)
http_archive(
  name = "com_googlesource_code_re2",
  sha256 = "50dce81b27ed69b3d6a03ac5c7d24845a53aa5b472ef4d5d41109d3935de4f0b",
  strip_prefix = "re2-9c9d64d9396af4d97e670b7828c2d770383810f4",
  urls = ["https://github.com/google/re2/archive/9c9d64d9396af4d97e670b7828c2d770383810f4.tar.gz"]
)

# Bazel (for build event protocol)
http_archive(
  name = "io_bazel",
  sha256 = "f07d55500307f1dfafc47203116e36f9d3fde98423db84673a393d3975c63935",
  strip_prefix = "bazel-98af5be784026cff466d4a4a090b60ca3255e963",
  urls = ["https://github.com/endobson/bazel/archive/98af5be784026cff466d4a4a090b60ca3255e963.tar.gz"],
)

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")
grpc_deps()

load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")
grpc_extra_deps()
