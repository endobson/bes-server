workspace(name = "bes_example")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

# GRPC
http_archive(
  name = "com_github_grpc_grpc",
  sha256 = "2fcb7f1ab160d6fd3aaade64520be3e5446fc4c6fa7ba6581afdc4e26094bd81",
  strip_prefix = "grpc-1.26.0",
  urls = ["https://github.com/grpc/grpc/archive/v1.26.0.tar.gz"]
)

# BoringSSL since grpc uses a spammy version without a sha256
git_repository(
    name = "boringssl",
    commit = "83da28a68f32023fd3b95a8ae94991a07b1f6c62",
    shallow_since = "1571438383 +0000",
    remote = "https://boringssl.googlesource.com/boringssl",
)

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps",)
grpc_deps()
# Extra libraries needed for grpc
load("@upb//bazel:workspace_deps.bzl", "upb_deps")
upb_deps()
load("@build_bazel_rules_apple//apple:repositories.bzl", "apple_rules_dependencies")
apple_rules_dependencies()

## Bazel (for build event protocol)
http_archive(
  name = "io_bazel",
  sha256 = "f07d55500307f1dfafc47203116e36f9d3fde98423db84673a393d3975c63935",
  strip_prefix = "bazel-98af5be784026cff466d4a4a090b60ca3255e963",
  urls = ["https://github.com/endobson/bazel/archive/98af5be784026cff466d4a4a090b60ca3255e963.tar.gz"],
)

# abseil-cpp (2019-03-14)
http_archive(
  name = "com_google_absl",
  urls = ["https://github.com/abseil/abseil-cpp/archive/7c7754fb3ed9ffb57d35fe8658f3ba4d73a31e72.zip"],
  strip_prefix = "abseil-cpp-7c7754fb3ed9ffb57d35fe8658f3ba4d73a31e72",
  sha256 = "71d00d15fe6370220b6685552fb66e5814f4dd2e130f3836fc084c894943753f",
)
