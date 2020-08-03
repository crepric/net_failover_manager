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

#ifndef NET_FAILOVER_MANAGER_NETCTL_INTERFACE_CHECKER
#define NET_FAILOVER_MANAGER_NETCTL_INTERFACE_CHECKER

#include <condition_variable>
#include <ctime>
#include <functional>
#include <glog/logging.h>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>

namespace net_failover_manager {

class InterfaceChecker {
public:
  // Interface statuses.
  typedef enum {
    UNKNOWN,   // Should only be uninitialized.
    HEALTHY,   // Interface can ping public IPs.
    UNHEALTHY, // Interface cannot ping public IPs.
  } InterfaceStatus;

  // Callback to be called when the status of an interface changes. Callback
  // will be called with name of the interface that changed, old status and
  // new status.
  typedef std::function<void(const std::string &, InterfaceStatus,
                             InterfaceStatus)>
      IfStatusChangedCallback;

  // Convert status into string for debugging purposes.
  static std::string InterfaceStatusAsString(InterfaceStatus interface_status) {
    switch (interface_status) {
    case UNKNOWN:
      return "UNKNOWN";
    case HEALTHY:
      return "HEALTHY";
    case UNHEALTHY:
      return "UNHEALTHY";
    }
    LOG(ERROR) << "Unknown status";
    return "N/A";
  }

  // Takes list of interfaces to be checked as string.
  explicit InterfaceChecker(const std::vector<std::string> &if_list,
                            IfStatusChangedCallback status_changed_cb);
  // Does not set a callback.
  explicit InterfaceChecker(const std::vector<std::string> &if_list);
  virtual ~InterfaceChecker();

  void RegisterIfStatusChangedCb(IfStatusChangedCallback if_status_changed_cb) {
    std::unique_lock<std::mutex> lock(cb_mutex_);
    status_changed_cb_ = if_status_changed_cb;
  }
  // Starts a separate thread for each interface to periodically test each
  // interface.
  bool StartChecks();
  // Stop checks for all interfaces.
  bool StopChecks();

  // Return the status of one of the interfaces. Return value has both status
  // and timestamp of the last check. Returns nullopt if interface is not known.
  std::optional<std::pair<InterfaceStatus, std::time_t>>
  CheckStatus(const std::string &if_name) {
    std::unique_lock<std::mutex> lock(mutex_);
    auto if_desc = interface_status_.find(if_name);
    if (if_desc != interface_status_.end()) {
      auto ret = std::make_pair((*if_desc).second.status,
                                (*if_desc).second.last_checked_at);
      return std::optional<std::pair<InterfaceStatus, std::time_t>>(ret);
    }
    return std::nullopt;
  };

  std::vector<std::string> InterfaceNames() const {
    std::unique_lock<std::mutex> lock(mutex_);
    std::vector<std::string> ret;
    for (const auto &entry : interface_status_) {
      ret.push_back(entry.first);
    }
    return ret;
  }

protected:
  // Delete copy and move constructors.
  InterfaceChecker(const InterfaceChecker &) = delete;
  InterfaceChecker &operator=(const InterfaceChecker &) = delete;

private:
  typedef struct {
    InterfaceStatus status;
    std::unique_ptr<std::thread> check_thread;
    std::time_t last_checked_at;
  } InterfaceDescriptor;

  mutable std::mutex mutex_;
  mutable std::condition_variable checks_loop_cond_;

  bool checks_ongoing_; // Protected by mutex_;

  // Stores the current status of the interface. Protected by mutex_.
  std::unordered_map<std::string, InterfaceDescriptor> interface_status_;
  // Set only at constructor.
  mutable std::mutex cb_mutex_; // Different mutex to avoid lock inversion.
  IfStatusChangedCallback status_changed_cb_;

}; // class Interface Checker.
} // namespace net_failover_manager
#endif // #ifndef NET_FAILOVER_MANAGER_NETCTL_INTERFACE_CHECKER
