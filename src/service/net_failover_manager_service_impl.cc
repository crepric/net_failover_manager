#include "net_failover_manager_service_impl.h"

#include <ctime>

namespace net_failover_manager {

NetworkConfigImpl::NetworkConfigImpl(RouteManager *rm, InterfaceChecker *ic)
    : rm_(rm), ic_(ic){};

grpc::Status NetworkConfigImpl::GetDefaultGw(grpc::ServerContext *context,
                                             const DefaultGwRequest *request,
                                             DefaultGwResponse *response) {
  std::unique_lock<std::mutex> lock(mutex_);
  auto gw = rm_->PrimaryDefaultGwInterface();
  if (gw.has_value()) {
    response->set_default_gw_interface(gw.value());
    return grpc::Status::OK;
  }
  return grpc::Status(grpc::StatusCode::NOT_FOUND,
                      "Could not identify default GW");
}

grpc::Status NetworkConfigImpl::GetIfStatus(grpc::ServerContext *context,
                                            const IfStatusRequest *request,
                                            IfStatusResponse *response) {
  std::unique_lock<std::mutex> lock(mutex_);
  auto interfaces = ic_->InterfaceNames();

  for (const auto &name : interfaces) {
    auto *if_status_response = response->add_interface_status();
    if_status_response->set_if_name(name);
    auto if_status = ic_->CheckStatus(name);
    if (if_status.has_value()) {
      if_status_response->set_status(
          InterfaceChecker::InterfaceStatusAsString(if_status.value().first));
      if_status_response->set_last_checked_at(
          std::asctime(std::localtime(&(if_status.value().second))));
    } else {
      LOG(ERROR) << "Could not retrieve status for interface: " << name;
    }
  }
  return grpc::Status::OK;
}

grpc::Status
NetworkConfigImpl::ForceNewGateway(grpc::ServerContext *context,
                                   const ForceNewGatewayRequest *request,
                                   ForceNewGatewayResponse *response) {
  rm_->SetDefaultGw(request->if_name());
  return grpc::Status::OK;
}
} // namespace net_failover_manager
