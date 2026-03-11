#include "DeviceService.h"
#include <iostream>
#include <regex>

DeviceService::DeviceService(DeviceManager& device_manager, NetworkScanner& network_scanner) :
    device_manager_(device_manager), network_scanner_(network_scanner) {}

void DeviceService::validate_subnet(const std::string& subnet){
    std::regex subnet_pattern(R"(^(\d{1,3}\.){3}\d{1,3}\/\d{1,2}$)");

    if (!std::regex_match(subnet, subnet_pattern)){
        throw ServiceException({
            400,
            "Invalid subnet format (ex: 192.168.1.0/24)"
        });
    }
}

NetworkInfo DeviceService::detect_network(){
    std::lock_guard lock(mtx);
    auto info = network_scanner_.get_local_network();
    if (!info.is_valid){
        throw ServiceException({
            500,
            "Could not detect local network"
        });
    }
    network_info_ = info;
    return info;
}

NetworkInfo DeviceService::get_network_info() const{
    std::lock_guard lock(mtx);
    if(!network_info_){
        throw ServiceException({
            404,
            "Network not detected. Try calling detect_network()"
        });
    }
    return *network_info_;
}

ScanNetworkResponse DeviceService::scan_network(const ScanNetworkRequest& req){
    std::string subnet = req.subnet;
    
    if (subnet.empty()){
        auto info = get_network_info();
        subnet = info.subnet_cidr;
    }
    else {
        validate_subnet(subnet);
    }

    device_manager_.clear_devices();
    std::vector<Device> found_devices = network_scanner_.scan_targets(subnet);

    for (const auto& device : found_devices){
        device_manager_.add_device(device);
    }

    return ScanNetworkResponse{
        req.subnet,
        found_devices,
        static_cast<int>(found_devices.size())
    };
}

std::vector<Device> DeviceService::get_all_devices(){
    return device_manager_.get_all_devices();
}

Device DeviceService::get_device(const std::string& id){
    auto device = device_manager_.get_device(id);

    if (!device){
        throw ServiceException({
            404,
            "Device with id " + id + " not found"
        });
    }

    return *device;
}

SetTargetResponse DeviceService::set_target(const SetTargetRequest& req){
    auto device = device_manager_.get_device(req.device_id);

    if (!device){
        throw ServiceException({
            404,
            "Cannot set target: device with id " + req.device_id + "not found"
        });
    }

    device_manager_.set_target(req.device_id);

    return SetTargetResponse{
        *device,
        "Target set to device with ip " + device->ip
    };
}

Device DeviceService::get_current_target(){
    auto target = device_manager_.get_target();

    if (!target){
        throw ServiceException({
            404,
            "No target selected"
        });
    }
    return *target;
}

void DeviceService::clear_target(){
    device_manager_.clear_target();
}