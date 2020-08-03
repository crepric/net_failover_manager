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

#include <functional>
#include <glog/logging.h>

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
    gw_interface_order_.push_back(if_name);
  }
}

void GatewayConfigManager::GwChangedCb(const std::string &new_gw) {
  LOG(INFO) << "New Gateway!!! " << new_gw;
}

void GatewayConfigManager::IfChangedCb(
    const std::string &if_name, InterfaceChecker::InterfaceStatus old_status,
    InterfaceChecker::InterfaceStatus new_status) {
  LOG(INFO) << "IF status changed: " << if_name << " went from "
            << InterfaceChecker::InterfaceStatusAsString(old_status)
            << " to: " << InterfaceChecker::InterfaceStatusAsString(new_status);
};

} // namespace net_failover_manager
