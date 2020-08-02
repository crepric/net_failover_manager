#ifndef NET_FAILOVER_MANAGER_SERVICE_SERVICE_IMPL
#define NET_FAILOVER_MANAGER_SERVICE_SERVICE_IMPL

#include "src/netctl/interface_checker.h"
#include "src/netctl/route_manager.h"
#include "src/proto/net_failover_manager_service.grpc.pb.h"

#include <grpcpp/grpcpp.h>
#include <string>
#include <thread>

namespace net_failover_manager {

class NetworkConfigImpl final : public NetworkConfig::Service {
public:
  NetworkConfigImpl(RouteManager *rm, InterfaceChecker *ic);
  ~NetworkConfigImpl() override{};

  grpc::Status GetDefaultGw(grpc::ServerContext *context,
                            const DefaultGwRequest *request,
                            DefaultGwResponse *response) override;

  grpc::Status GetIfStatus(grpc::ServerContext *context,
                           const IfStatusRequest *request,
                           IfStatusResponse *response) override;

  grpc::Status ForceNewGateway(grpc::ServerContext *context,
                               const ForceNewGatewayRequest *request,
                               ForceNewGatewayResponse *response) override;

private:
  mutable std::mutex mutex_;
  // Ownership remains with the parent.
  RouteManager *rm_;
  InterfaceChecker *ic_;

}; // class NetworkConfigImpl

} // namespace net_failover_manager

#endif // #ifndef NET_FAILOVER_MANAGER_SERVICE_SERVICE_IMPL
