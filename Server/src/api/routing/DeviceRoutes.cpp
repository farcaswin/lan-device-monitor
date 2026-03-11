#include "DeviceRoutes.h"

DeviceRoutes::DeviceRoutes(DeviceService& dev_srv) : device_service_(dev_srv) {}

void DeviceRoutes::register_routes(httplib::Server& srv){
    // GET /api/devices
    srv.Get("/api/devices",
        [this](const httplib::Request& req, httplib::Response& res){
            handle_get_all_devices(req, res);
        }
    );

    // GET /api/devices/:id
    srv.Get(R"(/api/devices/([^/]+))",
        [this](const httplib::Request& req, httplib::Response& res){
            handle_get_device(req, res);
        }
    );

    // POST /api/devices/target
    srv.Get("/api/devices/target",
        [this](const httplib::Request& req, httplib::Response& res){
            handle_set_target(req, res);
        }
    );

    // GET /api/devices/target
    srv.Get("/api/devices/target",
        [this](const httplib::Request& req, httplib::Response& res){
            handle_get_target(req, res);
        }
    );

    // DELETE /api/devices/target
    srv.Delete("/api/devices/target",
        [this](const httplib::Request& req, httplib::Response& res){
            handle_clear_target(req, res);
        }
    );
}

void DeviceRoutes::handle_get_all_devices(const httplib::Request& req, httplib::Response& res){
    try{
        auto devices = device_service_.get_all_devices();

        json arr = json::array();
        for (const auto& dev : devices){
            arr.push_back(device_to_json(dev));
        }

        res.status = 200;
        res.set_content(arr.dump(2), "application/json");
    }
}
