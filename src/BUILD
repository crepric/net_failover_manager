cc_binary(
  name = "net_failover_manager",
  srcs = ["net_failover_manager.cc"],
  deps = [
    "//src/netctl:gateway_config_manager_lib",
    "//src/netctl:interface_checker_lib",
    "//src/service:net_failover_manager_service_lib",
    "//src/netctl:route_manager_lib",
    "@com_github_grpc_grpc//:grpc++_reflection",
  ],
)
