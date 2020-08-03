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
