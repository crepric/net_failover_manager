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
#include <stdio.h>
#include <stdlib.h>
#include <array>
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>

namespace net_failover_manager {

namespace {

// Ping commands arguments.
const std::string kAddressToPing = "8.8.8.8";  // Google Public DNS.
const int kPingTimeout = 1;       // seconds, timeout to receive ping reply.
const int kPingDuration = 3;      // seconds, duration of ping command.
const float kPingInterval = 0.5;  // seconds, interval between pings.

// Interval between two ping commands.
const std::chrono::duration kIfCheckInterval = std::chrono::seconds(20);

// Ping packet loss threshold for healthy interface;
const int kPingPacketLossThreshold = 25;

InterfaceChecker::InterfaceStatus ParsePingResult(
    const std::string &ping_result, const std::string &if_name) {
  boost::char_separator<char> fn("\n");
  boost::tokenizer<boost::char_separator<char> > tokens(ping_result, fn);
  for (auto str = tokens.begin(); str != tokens.end(); ++str) {
    auto pos = (*str).find("packet loss");
    if (pos != std::string::npos) {
      // Line looks like this:
      // 4 packets transmitted, 4 received, 0% packet loss, time 503ms
      DLOG(INFO) << "Found stats line for interface " << if_name << ": "
                 << *str;
      std::string delimiters(",");
      std::vector<std::string> stats_elements;
      boost::split(stats_elements, *str, boost::is_any_of(delimiters));
      for (auto element : stats_elements) {
        // looking for the packet loss info.
        auto pl_pos = element.find("% packet loss");
        if (pl_pos != std::string::npos) {
          int pl_value = 100;  // default to 100% packet loss.
          try {
            pl_value = stoi(element.substr(0, pl_pos));
          } catch (const std::invalid_argument &ia) {
            LOG(ERROR) << "Failed to convert packet loss to int value from "
                       << element;
            return InterfaceChecker::UNKNOWN;
          }
          DLOG(INFO) << "Packet loss for " << if_name << " recorded at "
                     << pl_value;
          if (pl_value > kPingPacketLossThreshold) {
            LOG(WARNING) << "Packet loss for " << if_name
                         << " higher than threshold, at " << pl_value;
            return InterfaceChecker::UNHEALTHY;
          } else {
            return InterfaceChecker::HEALTHY;
          }
        }
      }
    }
  }
  return InterfaceChecker::UNKNOWN;
}

// Run a command in a subprocess and returns command output.
std::string exec(const char *cmd) {
  std::array<char, 256> buffer;
  std::string output;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    output += buffer.data();
  }
  return output;
}

// Tests interface connectivity by calling the external command ping.
// TODO: change this to an internal ICMP sender.
void TestPing(const std::string &interface,
              InterfaceChecker::InterfaceStatus *result) {
  std::stringstream command;
  command << "ping " << kAddressToPing << " -W " << kPingTimeout << " -w "
          << kPingDuration << " -i " << kPingInterval << " -I " << interface;
  DLOG(INFO) << "calling " << command.str() << "\n";
  auto ping_result = exec(command.str().c_str());
  DLOG(INFO) << "Ping output:";
  *result = ParsePingResult(ping_result, interface);
}
}  // namespace

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
            next_check += kIfCheckInterval;
            InterfaceStatus status;
            TestPing(interface_name, &status);
            {
              std::unique_lock<std::mutex> lock(mutex_);
              if (interface_status_[interface_name].status != status) {
                LOG(INFO) << "Status changed for " << interface_name << " from "
                          << InterfaceStatusAsString(
                                 interface_status_[interface_name].status)
                          << " to " << InterfaceStatusAsString(status);
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

}  // namespace net_failover_manager
