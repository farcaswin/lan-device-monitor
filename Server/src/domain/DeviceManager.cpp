#include "DeviceManager.h"
#include <algorithm>
#include <mutex>

void DeviceManager::add_device(const Device& device){
    std::unique_lock lock(mtx);

    devices[device.id] = device;
}

void DeviceManager::remove_device(const std::string& id){
    std::unique_lock lock(mtx);
    devices.erase(id);

    if (target_id && *target_id == id){
        target_id = std::nullopt;
    }
}

void DeviceManager::update_device(const Device& device){
    std::unique_lock lock(mtx);
    if (devices.count(device.id)){
        devices[device.id] = device;
    }
}

std::optional<Device> DeviceManager::get_device(const std::string& target_id) const {
    std::unique_lock lock(mtx);

    auto it = devices.find(target_id);
    if (it == devices.end()){
        return std::nullopt;
    }
    return it->second;
}

std::vector<Device> DeviceManager::get_all_devices() const {
    std::unique_lock lock(mtx);
    std::vector<Device> result;
    result.reserve(devices.size());
    for (const auto& [id, dev] : devices){
        result.push_back(dev);
    }
    return result;
}

void DeviceManager::clear_devices(){
    std::unique_lock lock(mtx);
    devices.clear();
    target_id = std::nullopt;
}

bool DeviceManager::set_target(const std::string& id){
    std::unique_lock lock(mtx);
    if (!devices.count(id)){
        return false;
    }
    target_id = id;
    return true;
}

void DeviceManager::clear_target(){
    std::unique_lock lock(mtx);
    target_id = std::nullopt;
}

std::optional<Device> DeviceManager::get_target() const {
    std::shared_lock lock(mtx);
    if (!target_id){
        return std::nullopt;
    }
    auto it = devices.find(*target_id);
    if (it == devices.end()){
        return std::nullopt;
    }
    return it->second;
}