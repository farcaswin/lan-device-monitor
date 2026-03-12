#include "DeviceRoutes.h"

DeviceRoutes::DeviceRoutes(DeviceService& dev_srv) : device_service_(dev_srv) {}

void DeviceRoutes::register_routes(httplib::Server& srv){
    // GET /api/devices
    srv.Get("/api/devices",
        [this](const httplib::Request& req, httplib::Response& res){
            handle_get_all_devices(req, res);
        }
    );

    // GET /api/devices/target
    srv.Get("/api/devices/target",
        [this](const httplib::Request& req, httplib::Response& res){
            handle_get_target(req, res);
        }
    );

    // POST /api/devices/target
    srv.Post("/api/devices/target",
        [this](const httplib::Request& req, httplib::Response& res){
            handle_set_target(req, res);
        }
    );

    // DELETE /api/devices/target
    srv.Delete("/api/devices/target",
        [this](const httplib::Request& req, httplib::Response& res){
            handle_clear_target(req, res);
        }
    );

    // GET /api/devices/:id 
    srv.Get(R"(/api/devices/([^/]+))",
        [this](const httplib::Request& req, httplib::Response& res){
            std::string id = req.matches[1];
            if (id == "target")
            {
                handle_get_target(req, res);
                return;
            }

            handle_get_device(req, res);
        }
    );
}

void DeviceRoutes::handle_get_all_devices(
    const httplib::Request&, httplib::Response& res)
{
    try {
        auto devices = device_service_.get_all_devices();

        json arr = json::array();
        for (const auto& d : devices)
            arr.push_back(device_to_json(d));

        res.status = 200;
        res.set_content(arr.dump(2), "application/json");
    } catch (const ServiceException& e) {
        send_error(res, e.error().http_status, e.error().message);
    }
}

void DeviceRoutes::handle_get_device(const httplib::Request& req, httplib::Response& res){
    std::string device_id = req.matches[1];

    try{
        auto device = device_service_.get_device(device_id);

        res.status = 200;
        res.set_content(device_to_json(device).dump(2), "application/json");
    } catch (const ServiceException& e){
        send_error(res, e.error().http_status, e.error().message);
    }
}

void DeviceRoutes::handle_set_target(const httplib::Request& req, httplib::Response& res){
    json body;
    try{
        body = json::parse(req.body);
    } catch (...){
        send_error(res, 400, "Invalid JSON body");
        return;
    }

    if (!body.contains("device_id") || !body["device_id"].is_string()){
        send_error(res, 400, "Missing or invalid field: device_id");
    }

    SetTargetRequest request{ body["device_id"].get<std::string>() };

    try{
        auto response = device_service_.set_target(request);

        res.status = 200;
        res.set_content(
            json{
                {"device", device_to_json(response.device)},
                {"message", response.message}
            }.dump(2), "application/json"
        );
    } catch (const ServiceException& e){
        send_error(res, e.error().http_status, e.error().message);
    }
}

void DeviceRoutes::handle_get_target(const httplib::Request&, httplib::Response& res){
    try{
        auto device = device_service_.get_current_target();

        res.status = 200;
        res.set_content(device_to_json(device).dump(2), "application/json");
    } catch (const ServiceException& e){
        send_error(res, e.error().http_status, e.error().message);
    }
}

void DeviceRoutes::handle_clear_target(const httplib::Request& req, httplib::Response& res){
    device_service_.clear_target();

    res.status = 200;
    res.set_content(json{{"message", "Target cleared"}}.dump(2), "application/json");
}

json DeviceRoutes::port_to_json(const OpenPort& p){
    return json{
        {"port_number", p.port_number},
        {"protocol", p.protocol},
        {"service", p.service}
    };
}

json DeviceRoutes::device_to_json(const Device& d){
    json ports = json::array();
    for (const auto& p : d.open_ports){
        ports.push_back(port_to_json(p));
    }

    return json{
        {"id", d.id},
        {"ip", d.ip},
        {"mac_address", d.mac_address},
        {"os_version", d.os_version},
        {"vendor", d.vendor},
        {"is_alive", d.is_alive},
        {"ports", ports},
    };
}

void DeviceRoutes::send_error(httplib::Response& res, int status, const std::string& message){
    res.status = status;
    res.set_content(json{
        {"error", message},
        {"status", status}
    }.dump(2), "application/json");
}
