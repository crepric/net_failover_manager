// This file is part of Net Failover Manager.
//
// Net Failover Manager is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Net Failover Manager is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Net Failover Manager.  If not, see
// <https://www.gnu.org/licenses/>.

#include "route_manager.h"

#include <arpa/inet.h>
#include <errno.h>
#include <glog/logging.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <boost/tokenizer.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace net_failover_manager {

namespace {
// Constants used to parse entries in the routing table.
static const char *kRoutingTablePath = "/proc/net/route";
static const int kIfNameOffset = 0;
static const int kDstAddressOffset = 1;
static const int kGwAddressOffset = 2;
static const int kMetricOffset = 6;

static constexpr std::chrono::duration kCheckInterval = std::chrono::seconds(5);
// Transforms an IP represented as a string representing an int into a boost
// IP Address object. Currently only works for IPv4.
boost::asio::ip::address MakeAddressFromIntAsStr(const std::string &s) {
  char buf[INET_ADDRSTRLEN];
  char *dummy;
  uint32_t addr_int = strtol(s.c_str(), &dummy, 16);
  in_addr address;
  address.s_addr = addr_int;
  return boost::asio::ip::make_address(
      inet_ntop(AF_INET, &address, buf, sizeof(buf)));
}

// Checks if IPv4 address represents default route.
bool isAnyV4Address(::boost::asio::ip::address addr) {
  return addr == boost::asio::ip::address_v4::any();
}

// Configures a routing entry struct for a default route, that can then be
// programmed with ioctl.
//
// Args:
//   if_name : the name of the interface this route refers to.
//   metric : the route priority (note that when checked through proc/net/route
//            the actual value is metric + 1
//   gw_addr : the address of the gateway
//   route : a pointer to the struct that will be configured. All existing data
//           will be erased.
void ConfigureRoute(char *if_name, int metric, in_addr_t gw_addr,
                    struct rtentry *route) {
  memset(route, 0, sizeof(*route));

  // Destination address for a default route is always 0.0.0.0
  struct sockaddr_in *dst_sockaddr;
  dst_sockaddr = (struct sockaddr_in *)&(route->rt_dst);
  dst_sockaddr->sin_family = AF_INET;
  dst_sockaddr->sin_addr.s_addr = INADDR_ANY;

  // Default routes mask is always 0.0.0.0
  struct sockaddr_in *rt_mask;
  rt_mask = (struct sockaddr_in *)&(route->rt_genmask);
  rt_mask->sin_family = AF_INET;
  rt_mask->sin_addr.s_addr = INADDR_ANY;

  // Destination GW must be reacheable from the selected interface
  struct sockaddr_in *gw_sockaddr;
  gw_sockaddr = (struct sockaddr_in *)&(route->rt_gateway);
  gw_sockaddr->sin_family = AF_INET;
  gw_sockaddr->sin_addr.s_addr = gw_addr;

  // Metric represents the route priority. Lower numbers mean higher priority.
  // The value set here will be displayed as metric+1 in /net/proc/route
  route->rt_metric = metric;
  route->rt_flags = RTF_UP | RTF_GATEWAY;
  route->rt_dev = if_name;
}

// Converts an IP address (v4 or v6) into a string.
// Args:
//   sa : IP address to be converted.
//   output : char* that will be filled with the IP string.
//   maxlen : lenght of the char array passed in with output.
void get_ip_str(const struct sockaddr *sa, char *output, size_t maxlen) {
  switch (sa->sa_family) {
    case AF_INET:
      inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr), output,
                maxlen);
      break;

    case AF_INET6:
      inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr), output,
                maxlen);
      break;

    default:
      strncpy(output, "Unknown AF", maxlen);
  }
}

std::ostream &operator<<(std::ostream &strm, rtentry route) {
  char rt_dst_str[16];
  char rt_gateway_str[16];
  char rt_genmask_str[16];
  get_ip_str(&route.rt_dst, rt_dst_str, /*maxlen=*/16);
  get_ip_str(&route.rt_gateway, rt_gateway_str, /*maxlen=*/16);
  get_ip_str(&route.rt_genmask, rt_genmask_str, /*maxlen=*/16);

  strm << "Rt DST: " << rt_dst_str << " - Rt GW: " << rt_gateway_str
       << " - Rt GenMask: " << rt_genmask_str
       << " - Rt Metric: " << route.rt_metric << " - Rt dev " << route.rt_dev;
  return strm;
}

bool AddRoute(struct rtentry &route) {
  int fd;
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  LOG(INFO) << "Adding route: " << route;

  if (ioctl(fd, SIOCADDRT, &route) < 0) {
    LOG(ERROR) << "Error while adding route for if: " << route.rt_dev
               << " - error: " << strerror(errno);
    return false;
  }

  LOG(INFO) << "Route added successfully";
  return true;
}

bool DeleteRoute(struct rtentry &route) {
  int fd;
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  LOG(INFO) << "Deleting route: " << route;
  if (ioctl(fd, SIOCDELRT, &route) < 0) {
    LOG(ERROR) << "Error while deleting route for if: " << route.rt_dev
               << " - error:" << strerror(errno);
    return false;
  }
  LOG(INFO) << "Route deleted successfully";
  return true;
}

std::ostream &operator<<(std::ostream &strm,
                         RouteManager::RoutingEntry &rtentry) {
  strm << rtentry.toString();
  return strm;
}

std::ostream &operator<<(std::ostream &strm,
                         const RouteManager::RoutingEntry &rtentry) {
  strm << rtentry.toString();
  return strm;
}

}  // namespace

RouteManager::RouteManager() : RouteManager(nullptr){};

RouteManager::RouteManager(GwChangedCallback default_gw_changed_cb)
    : checks_on_(false),
      route_check_thread_(nullptr),
      default_gw_changed_cb_(default_gw_changed_cb){};

void RouteManager::StartChecks() {
  std::unique_lock<std::mutex> lock(mutex_);
  checks_on_ = true;
  route_check_thread_ = std::make_unique<std::thread>([this] {
    while (true) {
      {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!checks_on_) {
          break;
        }
        SyncRoutingTable();
        checks_loop_cond_.wait_for(lock, kCheckInterval,
                                   [this] { return !checks_on_; });
        if (!checks_on_) {
          break;
        }
      }
      DLOG(INFO) << "=======\nRouting Table:\n=======";
      DLOG(INFO) << GetRoutingTableAsStr();
    }
  });
};

const std::string RouteManager::GetRoutingTableAsStr() const {
  std::string ret;
  std::unique_lock<std::mutex> lock(mutex_);
  for (const auto &entry : routing_entries_) {
    ret.append(entry.toString());
    ret.append("\n");
  }
  return ret;
};

Status RouteManager::SetDefaultGw(const std::string &new_gw_name) {
  // Step 1 locate current interface in list of default gws
  // The entire operation should be atomic.
  std::unique_lock<std::mutex> lock(mutex_);
  std::vector<RoutingEntry> gateways;
  for (const auto &entry : routing_entries_) {
    if (isAnyV4Address(entry.dst)) {
      gateways.push_back(entry);
    }
  }
  std::sort(gateways.begin(), gateways.end());
  LOG(INFO) << "Current order of gateways:";
  for (const auto &gw : gateways) {
    LOG(INFO) << gw.if_name;
  }
  if (gateways.empty()) {
    LOG(WARNING) << "There are no default gateways.";
    return Status(Status::NOT_FOUND, "There are no default gateways");
  }
  if (gateways[0].if_name == new_gw_name) {
    LOG(INFO) << "Interface " << new_gw_name << " is already the default GW";
    return Status(Status::NO_OP,
                  "Interface " + new_gw_name + " was already default.");
  }

  auto new_gw_routing_entry = std::find_if(
      gateways.begin(), gateways.end(), [&new_gw_name](const RoutingEntry &x) {
        return x.if_name == new_gw_name;
      });
  if (new_gw_routing_entry == gateways.end()) {
    LOG(WARNING) << "Interface " << new_gw_name
                 << " does not have a routing entry.";
    return Status(Status::NOT_FOUND, "Interface " + new_gw_name +
                                         " does not have a routing entry.");
  }

  LOG(INFO) << "DELETE: " << *new_gw_routing_entry;
  LOG(INFO) << "DELETE: " << gateways[0];

  struct rtentry old_route_for_old_gw;
  struct rtentry old_route_for_new_gw;
  struct rtentry new_route_for_old_gw;
  struct rtentry new_route_for_new_gw;
  int current_new_gw_metric = new_gw_routing_entry->metric + 1;
  int current_default_gw_metric = gateways[0].metric + 1;
  char *default_gw_if = const_cast<char *>(gateways[0].if_name.c_str());
  char *new_gw_if = const_cast<char *>(new_gw_routing_entry->if_name.c_str());
  in_addr_t current_gw_addr = inet_addr(gateways[0].gw.to_string().c_str());
  in_addr_t new_gw_addr =
      inet_addr(new_gw_routing_entry->gw.to_string().c_str());

  ConfigureRoute(default_gw_if, current_default_gw_metric, current_gw_addr,
                 &old_route_for_old_gw);
  ConfigureRoute(new_gw_if, current_new_gw_metric, new_gw_addr,
                 &old_route_for_new_gw);

  ConfigureRoute(default_gw_if, current_new_gw_metric, current_gw_addr,
                 &new_route_for_old_gw);
  ConfigureRoute(new_gw_if, current_default_gw_metric, new_gw_addr,
                 &new_route_for_new_gw);

  LOG(INFO) << "Reprogramming Network Routes";
  if (!DeleteRoute(old_route_for_new_gw)) {
    return Status(Status::UNKNOWN_ERROR,
                  "Could not delete old route for new gw");
  };
  if (!DeleteRoute(old_route_for_old_gw)) {
    return Status(Status::UNKNOWN_ERROR,
                  "Could not delete old route for old gw");
  }
  // Regardless of the success of each operation, we should try to add both
  // routes.
  bool res_add_old = AddRoute(new_route_for_old_gw);
  bool res_add_new = AddRoute(new_route_for_new_gw);
  if (!res_add_old || !res_add_new) {
    return Status(Status::UNKNOWN_ERROR,
                  "Could not successfuly add one of the routes.");
  }
  LOG(INFO) << "Reprogramming done";

  return Status::Ok();
}

bool RouteManager::SyncRoutingTable() {
  // Lock must be held by caller.
  routing_entries_.clear();
  // Read routing table to vector of strings
  std::ifstream route_in(kRoutingTablePath);
  if (!route_in) {
    LOG(ERROR) << "Could not open path " << kRoutingTablePath;
    return false;
  }
  std::vector<std::string> routing_lines;
  std::string line;
  // Skip first line, it's headers.
  std::getline(route_in, line);
  while (std::getline(route_in, line)) {
    // Empty lines shouldn not happen, but just in case.
    if (line.size() > 0) {
      routing_lines.push_back(line);
    }
  }
  route_in.close();
  typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
  boost::char_separator<char> sep{"\t"};
  for (auto line : routing_lines) {
    RoutingEntry new_entry;
    line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
    tokenizer tok{line, sep};
    int count = 0;
    for (const auto &t : tok) {
      switch (count) {
        case kIfNameOffset:
          new_entry.if_name = t;
          break;
        case kDstAddressOffset:
          new_entry.dst = MakeAddressFromIntAsStr(t);
          break;
        case kGwAddressOffset:
          new_entry.gw = MakeAddressFromIntAsStr(t);
          break;
        case kMetricOffset:
          new_entry.metric = stoi(t);
          break;
        default:
          break;
      }
      count++;
    }
    routing_entries_.push_back(new_entry);
  }
  auto missing_gateways = DetectMissingGateways();
  if (!missing_gateways.empty()) {
    for (auto entry : missing_gateways) {
      LOG(WARNING) << "Missing expected gateway from routing table:" << entry;
      // TODO(crepric): debug why this happens and figure out if we need to
      // re-run dhcp here, or if we should let it be handled with the other
      // GW changes logic. The problem is if the interface somehow goes
      // unhelthy and needs to be brought down and up again to be fixed, which
      // shouldn't happen.
    }
  }
  DetectPrimaryDefaultGwInterface();
  return true;
}

const std::unordered_set<std::string> RouteManager::DetectMissingGateways() {
  // Mutex must be locked by caller.
  std::unordered_set<std::string> ret = known_gateway_interfaces_;
  for (const auto &entry : routing_entries_) {
    if (isAnyV4Address(entry.dst)) {
      DLOG(INFO) << "Inserting known gateway " << entry.if_name;
      known_gateway_interfaces_.insert(entry.if_name);
      ret.erase(entry.if_name);
    }
  }
  return ret;
}

const std::string &RouteManager::DetectPrimaryDefaultGwInterface() {
  // Mutex must be locked by caller.
  int min_metric = INT_MAX;  // Lower priority number means higher priority.
  std::string ret;
  for (const auto &entry : routing_entries_) {
    if (entry.metric >= min_metric) {
      continue;
    }
    if (isAnyV4Address(entry.dst)) {
      if (entry.metric == min_metric) {
        // This shouldn't happen.
        LOG(ERROR) << "Two default routes with the same metric. Second one is: "
                   << entry;
      }
      min_metric = entry.metric;
      ret = entry.if_name;
    }
  }
  if (ret != current_default_interface_) {
    LOG(INFO) << "Default interface has changed from "
              << current_default_interface_ << " to: " << ret;
    current_default_interface_ = ret;
    std::unique_lock<std::mutex> cb_lock(cb_mutex_);
    if (default_gw_changed_cb_) {
      DLOG(INFO) << "Calling gw change callback.";
      auto cb = std::bind(default_gw_changed_cb_, current_default_interface_);
      // FIXME: if two changes happen very close to each other, they may be
      // scheduled in reverse order. This shouldn't happen since only one change
      // per check can happem and they are spaced off by several seconds, but an
      // event queue is a better solution.
      auto t = std::thread(cb);
      t.detach();
    }
  }
  return current_default_interface_;
}

}  // namespace net_failover_manager
