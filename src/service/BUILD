load("@rules_proto//proto:defs.bzl", "proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")
load("@com_github_grpc_grpc//bazel:python_rules.bzl", "py_proto_library")
load("@com_github_grpc_grpc//bazel:python_rules.bzl", "py_grpc_library")

cc_library(
  name="net_failover_manager_service_lib",
  srcs=["net_failover_manager_service_impl.cc"],
  hdrs=["net_failover_manager_service_impl.h"],
  visibility=["//src:__pkg__"],
  deps=[
    "//src/proto:net_failover_manager_service_cc_grpc",
    "//src/netctl:interface_checker_lib",
    "//src/netctl:route_manager_lib",
    "@com_github_grpc_grpc//:grpc++",
  ],
)
