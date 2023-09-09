#!/bin/sh
ulimit -n 4096
bazel build  --bes_backend= //:bes_server && bazel-bin/bes_server
