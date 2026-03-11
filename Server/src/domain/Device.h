#pragma once
#include <string>
#include <vector>

struct OpenPort{
    int port_number;
    std::string protocol;
    std::string service;
};

struct Device{
    std::string id;
    std::string ip;
    std::string mac_address;
    std::string name;
    std::string os_version;
    std::string vendor;

    std::vector<OpenPort> open_ports;
    bool is_alive;
}; 