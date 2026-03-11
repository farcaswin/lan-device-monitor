#include "DeviceManager.h"
#include "DeviceService.h"
#include "NetworkScanner.h"
#include "HttpSrv.h"
#include "DeviceRoutes.h"
#include <iostream>

int main() {
    DeviceManager device_manager;
    NetworkScanner network_scanner;
    DeviceService device_service(device_manager, network_scanner);

    std::cout << "Starting server";
    try{
        auto info = device_service.detect_network();
    } catch (const ServiceException& e){
        std::cerr << "Warning: " << e.what() << "\n";
    }

    HttpSrv server("0.0.0.0", 8080);
    server.add_route_group(std::make_unique<DeviceRoutes>(device_service));
    std::cout << "Server listening on http://localhost:8080\n";
    server.start();

    return 0;
}




