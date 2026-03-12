#include "ScanRoutes.h"

ScanRoutes::ScanRoutes(DeviceService& dev_srv) : device_service_(dev_srv) {}

void ScanRoutes::register_routes(httplib::Server& srv){
    srv.Post("/api/scan",
        [this](const httplib::Request& req, httplib::Response& res){
            handle_scan_network(req, res);
        }
    );

    srv.Get("/api/network",
        [this](const httplib::Request& req, httplib::Response& res){
            handle_get_network_info(req, res);
        }
    );
}

void ScanRoutes::handle_scan_network(const httplib::Request& req, httplib::Response& res){
    std::string subnet;

    if(!req.body.empty()){
        try{
            auto body = json::parse(req.body);

            if (body.contains("subnet") && body["subnet"].is_string()){
                subnet = body["subnet"].get<std::string>();
            }
        } catch (...){
            send_error(res, 400, "Invalid JSON body");
            return;
        }
    }

    try{
        ScanNetworkRequest request{ subnet };
        auto response = device_service_.scan_network(request);

        json devices_arr = json::array();
        for (const auto& dev : response.devices){
            devices_arr.push_back(json{
                {"id", dev.id},
                {"ip", dev.ip},
                {"mac_address", dev.mac_address},
                {"name", dev.name},
                {"os_version", dev.os_version},
                {"vendor", dev.vendor},
                {"is_alive", dev.is_alive}
            });
        }
        res.status = 200;
        res.set_content(json{
            {"subnet",      response.subnet},
            {"total_found", response.total_found},
            {"devices",     devices_arr}
        }.dump(2), "application/json");

    } catch (const ServiceException& e) {
        send_error(res, e.error().http_status, e.error().message);
    }
}

void ScanRoutes::handle_get_network_info(
    const httplib::Request&, httplib::Response& res)
{
    try {
        auto info = device_service_.get_network_info();

        res.status = 200;
        res.set_content(json{
            {"interface",  info.interface_name},
            {"local_ip",   info.local_ip},
            {"subnet",     info.subnet_cidr},
            {"gateway",    info.gateway},
            {"is_valid",   info.is_valid}
        }.dump(2), "application/json");

    } catch (const ServiceException& e) {
        send_error(res, e.error().http_status, e.error().message);
    }
}

void ScanRoutes::send_error(httplib::Response& res,
                             int status,
                             const std::string& message)
{
    res.status = status;
    res.set_content(json{
        {"error",  message},
        {"status", status}
    }.dump(2), "application/json");
}