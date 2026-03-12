#pragma once
#include "DeviceService.h"
#include "RouteGroup.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class DeviceRoutes : public RouteGroup{
public:
    explicit DeviceRoutes(DeviceService& dev_srv);
    void register_routes(httplib::Server& srv) override;

private:
    DeviceService& device_service_;

    // Handlers
    void handle_get_all_devices(const httplib::Request&, httplib::Response&);
    void handle_set_target(const httplib::Request&, httplib::Response&);
    void handle_get_target(const httplib::Request&, httplib::Response&);
    void handle_clear_target(const httplib::Request&, httplib::Response&);
    void handle_get_device(const httplib::Request&, httplib::Response&);

    // Serializare json
    static json device_to_json(const Device& dev);
    static json port_to_json(const OpenPort& port);

    static void send_error(httplib::Response& res, int status, const std::string& msg);
};