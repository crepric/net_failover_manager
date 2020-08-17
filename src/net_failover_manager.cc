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

#include <stdlib.h>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <grpcpp/grpcpp.h>
#include "src/netctl/gateway_config_manager.h"
#include "src/netctl/interface_checker.h"
#include "src/netctl/route_manager.h"
#include "src/service/net_failover_manager_service_impl.h"

using net_failover_manager::GatewayConfigManager;
using net_failover_manager::InterfaceChecker;
using net_failover_manager::RouteManager;

void RunServer(RouteManager *rm, InterfaceChecker *ic) {
  std::string address = "0.0.0.0";
  std::string port = "50051";
  std::string server_address = address + ":" + port;
  net_failover_manager::NetworkConfigImpl service(rm, ic);

  grpc::ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  std::vector<std::string> interfaces = {"eth1", "usb0"};
  InterfaceChecker ic(interfaces);
  RouteManager rm;
  GatewayConfigManager gm(&ic, &rm);
  gm.SetPreferredGatewayInterfaces(interfaces);
  LOG(INFO) << "Starting the interface checks";
  ic.StartChecks();
  rm.StartChecks();

  auto default_interface = rm.PrimaryDefaultGwInterface();
  if (default_interface.has_value()) {
    LOG(INFO) << "Default interface " << rm.PrimaryDefaultGwInterface().value();
  } else {
    LOG(WARNING) << "No default interface";
  }
  LOG(WARNING) << "\nSetting gw\n";
  LOG(WARNING) << "\nSetting gw done\n";

  RunServer(&rm, &ic);
  rm.StopChecks();
  ic.StopChecks();
}
