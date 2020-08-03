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
