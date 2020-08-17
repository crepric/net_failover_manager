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

#include "gateway_config_manager.h"

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <functional>

namespace net_failover_manager {

GatewayConfigManager::GatewayConfigManager(InterfaceChecker *ic,
                                           RouteManager *rm)
    : ic_(ic), rm_(rm) {
  ic_->RegisterIfStatusChangedCb(
      [this](const std::string &if_name,
             InterfaceChecker::InterfaceStatus old_status,
             InterfaceChecker::InterfaceStatus new_status) {
        IfChangedCb(if_name, old_status, new_status);
      });
  rm_->RegisterGwChangedCb(
      [this](const std::string &new_gw) { GwChangedCb(new_gw); });
}

void GatewayConfigManager::SetPreferredGatewayInterfaces(
    const std::vector<std::string> &interfaces) {
  std::unique_lock<std::mutex> lock(mutex_);
  gw_interface_order_.clear();
  LOG(INFO) << "Resetting preferred interfaces list.";
  for (const auto &if_name : interfaces) {
    LOG(INFO) << "Adding " << if_name;
    // TODO(crepric): check for duplicates.
    gw_interface_order_.push_back(if_name);
  }
}

void GatewayConfigManager::GwChangedCb(const std::string &new_gw) {
  LOG(INFO) << "New Gateway!!! " << new_gw;
  // TODO(crepric): there's a new gateway, unless it is the first one in the
  // preference list in a HEALTHY status, we should revert back to the correct
  // onw.
}

void GatewayConfigManager::IfChangedCb(
    const std::string &if_name, InterfaceChecker::InterfaceStatus old_status,
    InterfaceChecker::InterfaceStatus new_status) {
  if (old_status == new_status) {
    LOG(WARNING) << "Device " << if_name << " has not changed state, still "
                 << InterfaceChecker::InterfaceStatusAsString(old_status);
    return;
  }
  LOG(INFO) << "IF status changed: " << if_name << " went from "
            << InterfaceChecker::InterfaceStatusAsString(old_status)
            << " to: " << InterfaceChecker::InterfaceStatusAsString(new_status);
  // If new status is HEALTHY, check if the interface that became healthy is
  // higher in priority compared the the current gateway, if it is, switch
  // them over.
  auto current_gateway = rm_->PrimaryDefaultGwInterface();
  switch (new_status) {
    case InterfaceChecker::HEALTHY: {
      // The device has turned healthy, let's check if it must become the new
      // gateway.
      if (current_gateway.has_value() && current_gateway.value() == if_name) {
        LOG(INFO) << "New healthy interface " << if_name
                  << "is already preferred gateway, nothing to do.";
        return;
      }
      std::unique_lock<std::mutex> lock(mutex_);
      auto new_if_it = std::find(gw_interface_order_.begin(),
                                 gw_interface_order_.end(), if_name);
      if (new_if_it == gw_interface_order_.end()) {
        LOG(WARNING) << "Interface " << if_name
                     << " not in the preferred gateways list";
        return;
      } else {
        if (current_gateway.has_value()) {
          int new_if_priority = new_if_it - gw_interface_order_.begin();
          int old_if_priority =
              std::find(gw_interface_order_.begin(), gw_interface_order_.end(),
                        current_gateway.value()) -
              gw_interface_order_.begin();
          if (new_if_priority >= old_if_priority) {
            LOG(INFO) << "The new healthy interface is lower priority than the "
                         "current gateway. Skip.";
            return;
          }
          rm_->SetDefaultGw(if_name);
        }
      }
    } break;
    default:
      // For now let's treat all other cases as unhealthy, if the device was the
      // gateway, switch to an (healthy) alternative.
      {
        if (!current_gateway.has_value() ||
            current_gateway.value() != if_name) {
          LOG(INFO)
              << if_name
              << "is unhealthy but wasn't the default gateway, nothing to do.";
          return;
        }
        std::unique_lock<std::mutex> lock(mutex_);
        for (auto interface : gw_interface_order_) {
          auto if_status = ic_->CheckStatus(interface);
          if (if_status.has_value() &&
              if_status.value().first == InterfaceChecker::HEALTHY) {
            LOG(INFO) << "Interface "
                      << interface << " is healthy, switching gateway";
            rm_->SetDefaultGw(interface);
            return;
          }
        }
      }
      break;
  }
};

}  // namespace net_failover_manager
