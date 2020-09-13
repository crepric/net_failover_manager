# This file is part of Net Failover Manager.
#
# Net Failover Manager is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Net Failover Manager is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Net Failover Manager.  If not, see <https://www.gnu.org/licenses/>.

load("@rules_proto//proto:defs.bzl", "proto_library")
load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")

cc_library(
    name = "net_failover_manager_service_lib",
    srcs = ["net_failover_manager_service_impl.cc"],
    hdrs = ["net_failover_manager_service_impl.h"],
    visibility = ["//src:__pkg__"],
    deps = [
        "//src/netctl:interface_checker_lib",
        "//src/netctl:route_manager_lib",
        "//src/proto:net_failover_manager_service_cc_grpc",
        "@com_github_grpc_grpc//:grpc++",
    ],
)
