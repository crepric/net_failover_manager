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

#include "interface_checker.h"
#include <glog/logging.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

namespace net_failover_manager {

namespace {

const std::string kAddressToPing = "8.8.8.8"; // Google Public DNS.
const int kPingTimeout = 4; // seconds, argument passed to the external ping.
const std::chrono::duration kPingInterval = std::chrono::seconds(2);

// Tests interface connectivity by calling the external command ping.
// TODO: change this to an internal ICMP sender.
void TestPing(const std::string &interface,
              InterfaceChecker::InterfaceStatus *result) {
  std::stringstream command;
  command << "ping " << kAddressToPing << " -c 1 -W " << kPingTimeout << " -I "
          << interface << " > /dev/null";
  DLOG(INFO) << "calling " << command.str() << "\n";
  int ping_result = system(command.str().c_str());
  // Ping return status is 0 if ping successful.
  if (ping_result == 0) {
    *result = InterfaceChecker::HEALTHY;
  } else {
    *result = InterfaceChecker::UNHEALTHY;
  }
}
} // namespace

InterfaceChecker::InterfaceChecker(const std::vector<std::string> &if_list,
                                   IfStatusChangedCallback status_changed_cb)
    : checks_ongoing_(false), status_changed_cb_(status_changed_cb) {
  for (const auto &if_name : if_list) {
    interface_status_[if_name].status = UNKNOWN;
  }
}

InterfaceChecker::InterfaceChecker(const std::vector<std::string> &if_list)
    : InterfaceChecker(if_list, nullptr){};
InterfaceChecker::~InterfaceChecker() { StopChecks(); }

bool InterfaceChecker::StartChecks() {
  std::unique_lock<std::mutex> lock(mutex_);
  checks_ongoing_ = true;
  for (const auto &interface_entry : interface_status_) {
    const auto &interface_name = interface_entry.first;
    interface_status_[interface_name].check_thread.reset(
        new std::thread([interface_name, this] {
          while (true) {
            std::time_t timestamp = std::time(nullptr);
            std::chrono::system_clock::time_point next_check =
                std::chrono::system_clock::now();
            next_check += kPingInterval;
            InterfaceStatus status;
            TestPing(interface_name, &status);
            {
              std::unique_lock<std::mutex> lock(mutex_);
              if (interface_status_[interface_name].status != status) {
                std::unique_lock<std::mutex> lock(cb_mutex_);
                if (status_changed_cb_) {
                  // FIXME: It is possible that more than one interface changes
                  // status at the same time, generating a rapid succession of
                  // callbacks that could be processed in any order at the
                  // receiver side, generating possible races.
                  auto cb = std::bind(status_changed_cb_, interface_name,
                                      interface_status_[interface_name].status,
                                      status);
                  auto t = std::thread(cb);
                  t.detach();
                }
                interface_status_[interface_name].status = status;
              }
              interface_status_[interface_name].last_checked_at = timestamp;
              LOG_EVERY_N(INFO, 10)
                  << "Checked " << interface_name
                  << " - status: " << InterfaceStatusAsString(status)
                  << " - last checked at: "
                  << std::asctime(std::localtime(&timestamp));
              checks_loop_cond_.wait_until(lock, next_check,
                                           [this] { return !checks_ongoing_; });
              if (!checks_ongoing_) {
                break;
              }
            }
          }
        }));
  }
  return true;
}

bool InterfaceChecker::StopChecks() {
  std::vector<std::thread *> threads_to_join;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!checks_ongoing_) {
      LOG(WARNING) << "StopChecks called twice.";
      return false;
    }
    checks_ongoing_ = false;
    for (auto &interface_entry : interface_status_) {
      threads_to_join.push_back(interface_entry.second.check_thread.release());
    }
    checks_loop_cond_.notify_all();
    // We empty this while holding the lock, and threads change status only if
    // already holding the lock. If they do acquire it later, the
    // checks_ongoinh_ variable will be false already and no access to the
    // interface will be performed. No race condidtion should be possible.
    interface_status_.clear();
  }
  for (auto *thread : threads_to_join) {
    thread->join();
    // Threads where released earlier.
    delete thread;
  }
  return true;
}

} // namespace net_failover_manager
