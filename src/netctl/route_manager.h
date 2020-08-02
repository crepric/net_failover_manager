// Monitors the system routing table periodically, checks for the highest
// priority default gateway, and optionally triggers a callback if default gw
// has changed.

#ifndef NET_FAILOVER_MANAGER_NETCTL_ROUTE_MANAGER
#define NET_FAILOVER_MANAGER_NETCTL_ROUTE_MANAGER

#include "src/lib/status.h"
#include <boost/asio/ip/address.hpp>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

namespace net_failover_manager {

class RouteManager {
public:
  // Holds relevant details for each routing entry.
  typedef struct RoutingEntry {
    std::string if_name;
    boost::asio::ip::address dst;
    boost::asio::ip::address gw;
    int metric;

    const std::string toString() const {
      return "If: " + if_name + " - Dst: " + dst.to_string() +
             " - Gw: " + gw.to_string() +
             " - Metric: " + std::to_string(metric);
    }

    bool operator<(const struct RoutingEntry &other) const {
      return (metric < other.metric);
    }

  } RoutingEntry;

  // Callback gets called if default gateway is changed. It will always be
  // called at least once at the beginning of execution when the routing table
  // is read for the first time.
  typedef std::function<void(const std::string &)> GwChangedCallback;

  // Default constructor does not specify a callback.
  RouteManager();
  // Callback must outlive this object.
  explicit RouteManager(GwChangedCallback default_gw_changed_cb);
  virtual ~RouteManager() {
    StopChecks();
    // FIXME: not thread safe.
    route_check_thread_->join();
  };

  void RegisterGwChangedCb(GwChangedCallback default_gw_changed_cb) {
    std::unique_lock<std::mutex> lock(cb_mutex_);
    default_gw_changed_cb_ = default_gw_changed_cb;
  }

  // Start/Stop periodic reads of the routing table and notifies callback of
  // default gw changes if a callback is specified.
  // Acquire lock.
  void StartChecks();
  void StopChecks() {
    std::unique_lock<std::mutex> lock(mutex_);
    checks_on_ = false;
    checks_loop_cond_.notify_all();
  }

  // Returns a string representing the routing table, one line for each entry.
  // Acquires lock.
  const std::string GetRoutingTableAsStr() const;
  std::optional<std::string> PrimaryDefaultGwInterface() const {
    return current_default_interface_;
  }

  // Reorganizes the entries of the existing gateway interfaces so that the
  // one specified in the argument becomes the preferred one.
  Status SetDefaultGw(const std::string &new_gw_name);

protected:
  // Delete copy and move constructors.
  RouteManager(const RouteManager &) = delete;
  RouteManager &operator=(const RouteManager &) = delete;

private:
  // Must be called with lock held.
  bool SyncRoutingTable();
  // Must be called with lock held.
  const std::string &DetectPrimaryDefaultGwInterface();

  mutable std::mutex mutex_;
  mutable std::condition_variable checks_loop_cond_;
  bool checks_on_;

  // Stores the current entries for the routing table. Protected by mutex_.
  std::vector<RoutingEntry> routing_entries_;
  // Highest priority (lowest number in the routing table) route for
  // a default Gateway. Protected by mutex_.
  std::string current_default_interface_;
  // Stores the thread that periodically reads the routing table and keeps it in
  // sync.
  std::unique_ptr<std::thread> route_check_thread_;
  // If set, this callback is called every time a default gateway interface
  // changes.
  mutable std::mutex cb_mutex_; // Dedicated mutex to avoid lock inversion.
  GwChangedCallback default_gw_changed_cb_;

}; // class RouteManager

} // namespace net_failover_manager

#endif // #ifndef NET_FAILOVER_MANAGER_NETCTL_ROUTE_MANAGER
