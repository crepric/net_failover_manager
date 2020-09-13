#include <chrono>
#include <iostream>
#include <thread>

#include <gflags/gflags.h>
#include <glog/logging.h>
#include <grpcpp/grpcpp.h>
#include "src/proto/net_failover_manager_service.grpc.pb.h"

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  std::unique_ptr<net_failover_manager::NetworkConfig::Stub> stub_;
  auto channel = grpc::CreateChannel("localhost:50051",
                                     grpc::InsecureChannelCredentials());
  stub_ = net_failover_manager::NetworkConfig::NewStub(channel);
  while (true) {
    grpc::ClientContext context;
    net_failover_manager::DefaultGwRequest request;
    net_failover_manager::DefaultGwResponse response;
    grpc::Status status = stub_->GetDefaultGw(&context, request, &response);
    std::cout << response.default_gw_interface() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
  }
}
