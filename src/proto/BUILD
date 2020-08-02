load("@rules_proto//proto:defs.bzl", "proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")
load("@com_github_grpc_grpc//bazel:python_rules.bzl", "py_proto_library")
load("@com_github_grpc_grpc//bazel:python_rules.bzl", "py_grpc_library")

proto_library(
  name = "net_failover_manager_service_proto",
  srcs = ["net_failover_manager_service.proto"],
)

cc_proto_library(
  name = "net_failover_manager_service_cc_proto",
  deps = [
    "net_failover_manager_service_proto"
  ],
)

cc_grpc_library(
  name = "net_failover_manager_service_cc_grpc",
  srcs = [":net_failover_manager_service_proto"],
  visibility = ["//src/service:__pkg__"],
  grpc_only = True,
  deps = [
    ":net_failover_manager_service_cc_proto",
  ],
)

py_proto_library(
  name = "net_failover_manager_service_py_proto",
  deps = [
    "net_failover_manager_service_proto"
  ],
)

py_grpc_library(
  name = "net_failover_manager_service_py_grpc",
  srcs = [":net_failover_manager_service_proto"],
  visibility = ["//:__subpackages__"],
  deps = [
    ":net_failover_manager_service_py_proto",
  ],
)
