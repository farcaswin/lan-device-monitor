#pragma once
#include "DeviceManager.h"
#include "NetworkScanner.h"
#include "NetworkInfo.h"
#include <string>
#include <vector>
#include <stdexcept>

struct ServiceError {
    int http_status;
    std::string message;
};

class ServiceException : public std::exception {
public: 
    explicit ServiceException(ServiceError err) : error_(std::move(err)) {}

    const ServiceError& error() const { return error_; }
    const char* what() const noexcept override {
        return error_.message.c_str();
    }

private:
    ServiceError error_;
};

struct ScanNetworkRequest{
    std::string subnet;
};

struct ScanNetworkResponse{
    std::string subnet;
    std::vector<Device> devices;
    int total_found;
};

struct SetTargetRequest{
    std::string device_id;
};

struct SetTargetResponse{
    Device device;
    std::string message;
};


class DeviceService{
public:
    DeviceService(DeviceManager& device_manager, NetworkScanner& network_scanner);

    NetworkInfo detect_network();
    NetworkInfo get_network_info() const;

    ScanNetworkResponse scan_network(const ScanNetworkRequest& req);
    
    std::vector<Device> get_all_devices();
    
    Device get_device(const std::string& device_id);

    SetTargetResponse set_target(const SetTargetRequest& req);

    Device get_current_target();

    void clear_target();
private: 
    DeviceManager& device_manager_;
    NetworkScanner& network_scanner_;

    std::optional<NetworkInfo> network_info_;
    mutable std::mutex mtx;

    void validate_subnet(const std::string& subnet);
};