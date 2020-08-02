load("@rules_python//python:defs.bzl", "py_binary")

filegroup(
  name = "html_templates",
  srcs = glob([
    "templates/*.html",
  ]),
)

filegroup(
  name = "static_resources",
  srcs = glob([
    "resources/js/*",
    "resources/css/*",
  ]),
)


py_binary(
  name = "net_failover_manager_fe",
  srcs = ["net_failover_manager_fe.py"],
  data = [":html_templates",
          ":static_resources"],
  deps = [
    "//src/proto:net_failover_manager_service_py_grpc",
    "@com_github_abseil_abseil_py//absl:app",
    "@com_github_abseil_abseil_py//absl/flags",
    "@com_github_abseil_abseil_py//absl/logging",
    "@rules_python//python/runfiles",
    "@com_github_grpc_grpc//src/python/grpcio/grpc:grpcio",
  ]
)
