// Monitors the status of the current routing table and interfaces, and if
// necessary, triggers a change in the default interface.

#ifndef NET_FAILOVER_MANAGER_NETCTL_GATEWAY_CONFIG_MANAGER
#define NET_FAILOVER_MANAGER_NETCTL_GATEWAY_CONFIG_MANAGER

#include <string>
#include <thread>
#include <vector>

#include "interface_checker.h"
#include "route_manager.h"

namespace net_failover_manager {

class GatewayConfigManager {
public:
  GatewayConfigManager(InterfaceChecker *ic, RouteManager *rm);
  virtual ~GatewayConfigManager() {}
  // Sets the list of preferred gateway interfaces based on the list passed in
  // as an argument.
  void
  SetPreferredGatewayInterfaces(const std::vector<std::string> &interfaces);

protected:
  // Delete copy and move constructors.
  GatewayConfigManager(const GatewayConfigManager &) = delete;
  GatewayConfigManager &operator=(const GatewayConfigManager &) = delete;

private:
  mutable std::mutex mutex_;
  // Stores the list of preferred gateway devices, in decreasing order of
  // preference. Protected by mutex_.
  std::vector<std::string> gw_interface_order_;

  // The two callback functions that are called when the network status changes.
  void GwChangedCb(const std::string &new_gw);
  void IfChangedCb(const std::string &if_name,
                   InterfaceChecker::InterfaceStatus old_status,
                   InterfaceChecker::InterfaceStatus new_status);

  // Set only at constructor, classes are thread safe, no mutex needed.
  InterfaceChecker *ic_;
  RouteManager *rm_;
}; // class GatewayConfigManager

} // namespace net_failover_manager

#endif // #ifndef NET_FAIOLVER_MANAGER_NETCTL_GATEWAY_CONFIG_MANAGER
