cc_library(
  name = "interface_checker_lib",
  srcs = ["interface_checker.cc"],
  hdrs = ["interface_checker.h"],
  visibility = ["//src:__subpackages__"],
  deps = [
    "@boost//:fiber",
    "@boost//:thread",
    "//external:glog",
    "//external:gflags",
  ],
)

cc_library(
  name = "route_manager_lib",
  srcs = ["route_manager.cc"],
  hdrs = ["route_manager.h"],
  visibility = ["//src:__subpackages__"],
  deps = [
    "//src/lib:status_lib",
    "@boost//:fiber",
    "@boost//:asio",
    "@boost//:thread",
    "//external:glog",
  ],
)

cc_library(
  name = "gateway_config_manager_lib",
  srcs = ["gateway_config_manager.cc"],
  hdrs = ["gateway_config_manager.h"],
  visibility = ["//src:__subpackages__"],
  deps = [
    ":interface_checker_lib",
    ":route_manager_lib",
    "//external:glog",
    "//external:gflags",
  ],
)

