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

# Dependencies for Google flags and Google logging library.
http_archive(
  name = "com_github_gflags_gflags",
  urls = [
    "https://github.com/gflags/gflags/archive/v2.2.2.tar.gz"
  ],
  sha256 = "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf",
  strip_prefix = "gflags-2.2.2",
)

http_archive(
  name = "com_github_glog_glog",
  urls = [
    "https://github.com/google/glog/archive/v0.4.0.tar.gz"
  ],
  sha256 = "f28359aeba12f30d73d9e4711ef356dc842886968112162bc73002645139c39c",
  strip_prefix = "glog-0.4.0",
)

bind(
    name = "glog",
    actual = "@com_github_glog_glog//:glog",
)

bind(
    name = "gflags",
    actual = "@com_github_gflags_gflags//:gflags",
)

# Dependency for C++ Boost library
git_repository(
    name = "com_github_nelhage_rules_boost",
    commit = "1e3a69bf2d5cd10c34b74f066054cd335d033d71",
    remote = "https://github.com/nelhage/rules_boost",
    shallow_since = "1591047380 -0700",
)

load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")
boost_deps()

# Dependency for GRPC
http_archive(
    name = "com_github_grpc_grpc",
    urls = [
#  This version of GRPC does not compile on raspberry, the "--linkopt=-latomic"
#  otpion is not honored by bazel.
#       "https://github.com/grpc/grpc/archive/v1.30.0.tar.gz",
        "https://github.com/grpc/grpc/archive/v1.26.0.tar.gz",
    ],
    sha256 = "2fcb7f1ab160d6fd3aaade64520be3e5446fc4c6fa7ba6581afdc4e26094bd81",
    strip_prefix = "grpc-1.26.0",
)

load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")
grpc_deps()

#Not mentioned in official docs... mentioned here https://github.com/grpc/grpc/issues/20511
load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")
grpc_extra_deps()

# Frontend dependencies.

http_archive(
  name = "rules_python",
  url = "https://github.com/bazelbuild/rules_python/releases/download/0.0.2/rules_python-0.0.2.tar.gz",
  strip_prefix = "rules_python-0.0.2",
  sha256 = "b5668cde8bb6e3515057ef465a35ad712214962f0b3a314e551204266c7be90c",
)

http_archive(
  name = "com_github_abseil_abseil_py",
  urls = [
    "https://github.com/abseil/abseil-py/archive/pypi-v0.9.0.tar.gz",
  ],
  sha256 = "603febc9b95a8f2979a7bdb77d2f5e4d9b30d4e0d59579f88eba67d4e4cc5462",
  strip_prefix = "abseil-py-pypi-v0.9.0",
)
