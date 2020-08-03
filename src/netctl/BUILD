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

cc_library(
    name = "interface_checker_lib",
    srcs = ["interface_checker.cc"],
    hdrs = ["interface_checker.h"],
    visibility = ["//src:__subpackages__"],
    deps = [
        "//external:gflags",
        "//external:glog",
        "@boost//:fiber",
        "@boost//:thread",
    ],
)

cc_library(
    name = "route_manager_lib",
    srcs = ["route_manager.cc"],
    hdrs = ["route_manager.h"],
    visibility = ["//src:__subpackages__"],
    deps = [
        "//external:glog",
        "//src/lib:status_lib",
        "@boost//:asio",
        "@boost//:fiber",
        "@boost//:thread",
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
        "//external:gflags",
        "//external:glog",
    ],
)
