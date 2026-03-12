#pragma once
#include "RouteGroup.h"
#include "DeviceService.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ScanRoutes : public RouteGroup {
public:
    explicit ScanRoutes(DeviceService& dev_srv);
    void register_routes(httplib::Server& srv) override;

private:
    DeviceService& device_service_;

    void handle_scan_network(const httplib::Request& req, httplib::Response& res);
    void handle_get_network_info(const httplib::Request& req, httplib::Response& res);

    static void send_error(httplib::Response& res, int status, const std::string& message);
};