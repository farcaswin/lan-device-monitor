#pragma once

#include "Device.h"
#include <string>
#include <unordered_map>
#include <optional>
#include <shared_mutex>
#include <memory>

class DeviceManager{
public:
    // CRUD methods
    void add_device(const Device& device);
    void remove_device(const std::string& id);
    void update_device(const Device& device);

    std::optional<Device> get_device(const std::string& target_id) const;
    std::vector<Device> get_all_devices() const;
    void clear_devices();

    bool set_target(const std::string& target_id);
    void clear_target();
    std::optional<Device> get_target() const;
private:
    mutable std::shared_mutex mtx;

    std::unordered_map<std::string, Device> devices;
    std::optional<std::string> target_id;
};