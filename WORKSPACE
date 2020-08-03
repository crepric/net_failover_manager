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



workspace(name = "net_failover_manager")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

git_repository(
    name = "com_github_gflags_gflags",
    remote = "http://github.com/gflags/gflags.git",
    tag = "v2.2.2",
)

git_repository(
    name = "com_github_nelhage_rules_boost",
    #commit = "1e3a69bf2d5cd10c34b74f066054cd335d033d71",
    branch = "master",
    remote = "https://github.com/nelhage/rules_boost",
    #shallow_since = "1591047380 -0700",
)

load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")
boost_deps()

git_repository(
    name = "com_github_glog_glog",
    tag = "v0.4.0",
    remote = "https://github.com/google/glog.git",
)

bind(
    name = "glog",
    actual = "@com_github_glog_glog//:glog",
)

bind(
    name = "gflags",
    actual = "@com_github_gflags_gflags//:gflags",
)

# This version of GRPC does not compile on raspberry, the "--linkopt=-latomic"
# otpion is not honored by bazel.
#http_archive(
#    name = "com_github_grpc_grpc",
#    urls = [
#        "https://github.com/grpc/grpc/archive/v1.30.0.tar.gz",
#    ],
#    strip_prefix = "grpc-1.30.0",
#)

http_archive(
    name = "com_github_grpc_grpc",
    urls = [
        "https://github.com/grpc/grpc/archive/v1.26.0.tar.gz",
    ],
    strip_prefix = "grpc-1.26.0",
)

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")

grpc_deps()

#Not mentioned in official docs... mentioned here https://github.com/grpc/grpc/issues/20511
load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")
grpc_extra_deps()


http_archive(
  name = "rules_python",
  url = "https://github.com/bazelbuild/rules_python/releases/download/0.0.2/rules_python-0.0.2.tar.gz",
  strip_prefix = "rules_python-0.0.2",
  sha256 = "b5668cde8bb6e3515057ef465a35ad712214962f0b3a314e551204266c7be90c",
)

git_repository(
  name = "com_github_abseil_abseil_py",
  tag = "pypi-v0.9.0",
  remote = "https://github.com/abseil/abseil-py.git",
)
