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
