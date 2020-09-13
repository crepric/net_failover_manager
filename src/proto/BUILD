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
load("@com_github_grpc_grpc//bazel:python_rules.bzl", "py_proto_library")
load("@com_github_grpc_grpc//bazel:python_rules.bzl", "py_grpc_library")

proto_library(
        name = "net_failover_manager_service_proto",
        srcs = ["net_failover_manager_service.proto"],
        )

cc_proto_library(
        name = "net_failover_manager_service_cc_proto",
        deps = [
        "net_failover_manager_service_proto",
        ],
        )

cc_grpc_library(
        name = "net_failover_manager_service_cc_grpc",
        srcs = [":net_failover_manager_service_proto"],
        grpc_only = True,
        visibility = ["//src/service:__pkg__", "//src/gw_check_cli:__pkg__"],
        deps = [
        ":net_failover_manager_service_cc_proto",
        ],
        )

py_proto_library(
        name = "net_failover_manager_service_py_proto",
        deps = [
        "net_failover_manager_service_proto",
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
