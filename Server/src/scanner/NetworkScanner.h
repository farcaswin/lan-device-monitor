#pragma once
#include <string>
#include <vector>
#include <NetworkInfo.h>
#include <Device.h>

class NetworkScanner {
public:
    NetworkInfo get_local_network();

    std::vector<Device> scan_targets(const std::string& subnet);

    std::vector<OpenPort> scan_ports(const std::string& target_ip,
                            const std::string& port_range);
private:
    

    static std::string exec_cmd(const std::string& cmd);
    
    

    static std::string extract_attr(const std::string& line, const std::string& attr);

    static std::string convert_device_id(const std::string& ip);

    std::vector<Device> parse_hosts_xml(const std::string& xml);
    std::vector<OpenPort> parse_ports_xml(const std::string& xml);
    std::string compute_subnet_cidr(const std::string& ip, int prefix);
};