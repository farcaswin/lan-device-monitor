#include "HttpSrv.h"
#include <iostream>
#include <httplib.h>

HttpSrv::HttpSrv(const std::string& host, int port) : host_(host), port_(port) {}

void HttpSrv::add_route_group(std::unique_ptr<RouteGroup> group){
    route_groups_.push_back(std::move(group));
}

void HttpSrv::setup_global_handlers(){
    server_.set_pre_routing_handler(
        [](const httplib::Request& req, httplib::Response& res)
        {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type");

            if (req.method == "OPTIONS"){
                res.status = 204;
                return httplib::Server::HandlerResponse::Handled;
            }
            return httplib::Server::HandlerResponse::Unhandled;
        } 
    );

    // 404
    server_.set_error_handler(
        [](const httplib::Request& req, httplib::Response& res)
        {
            res.set_content(
                R"({"error": "Not found", "path": ")" + req.path + R"("})", "application/json"
            );
        }
    );

}

void HttpSrv::start(){
    setup_global_handlers();

    for (auto& group : route_groups_){
        group->register_routes(server_);
    }

    server_.listen(host_.c_str(), port_);
}

void HttpSrv::stop(){   
    server_.stop();
}