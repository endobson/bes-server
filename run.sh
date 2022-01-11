#!/bin/sh
ulimit -n 4096
bazel build  --bes_backend= --cxxopt=--std=c++17 //:bes_server && \
  bazel-bin/bes_server
