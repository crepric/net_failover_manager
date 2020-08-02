#include <chrono>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <thread>
#include <vector>

#include "src/netctl/gateway_config_manager.h"
#include "src/netctl/interface_checker.h"
#include "src/netctl/route_manager.h"
#include "src/service/net_failover_manager_service_impl.h"
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <grpcpp/grpcpp.h>

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
  std::vector<std::string> interfaces = {"eth1", "usb0", "eth0"};
  InterfaceChecker ic(interfaces);
  RouteManager rm;
  GatewayConfigManager gm(&ic, &rm);
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
