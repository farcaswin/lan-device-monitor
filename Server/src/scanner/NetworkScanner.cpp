#include "NetworkScanner.h"
#include <arpa/inet.h>
#include <cstdio>
#include <string>
#include <array>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <regex>

std::string NetworkScanner::exec_cmd(const std::string& cmd){
    std::array<char, 4096> buffer;
    std::string result;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe)
        throw std::runtime_error("popen() failed.");

    while(fgets(buffer.data(), buffer.size(), pipe) != nullptr){
        result += buffer.data();
    }

    int exit_code = pclose(pipe);
    if (exit_code != 0){
        std::cerr << "nmap exit code: " << exit_code << "\n";
    }
    return result;
}

std::string NetworkScanner::extract_attr(const std::string& line, const std::string& attr){
    std::string search = attr + "=\"";
    size_t start = line.find(search);
    if (start == std::string::npos) return "";

    start += search.size();

    size_t end = line.find("\"", start);
    if (end == std::string::npos) return "";

    return line.substr(start, end - start);
}

std::string NetworkScanner::convert_device_id(const std::string& ip){
    std::string id = "dev-" + ip;

    std::replace(id.begin(), id.end(), '.', '-');
    return id;
}

std::vector<Device> NetworkScanner::parse_hosts_xml(
    const std::string& xml)
{
    std::vector<Device> devices;

    std::istringstream stream(xml);
    std::string line;

    Device  current;
    bool    in_host = false;

    while (std::getline(stream, line)) {

        if (line.find("<host ") != std::string::npos ||
            line.find("<host>") != std::string::npos)
        {
            in_host = true;
            current = Device{};
            continue;
        }

        if (line.find("</host>") != std::string::npos) {
            if (in_host && !current.ip.empty() && current.is_alive)
                devices.push_back(current);

            in_host = false;
            continue;
        }

        if (!in_host) continue;

        if (line.find("<status") != std::string::npos) {
            current.is_alive =
                line.find("state=\"up\"") != std::string::npos;
        }

        if (line.find("<address") != std::string::npos) {
            if (line.find("addrtype=\"ipv4\"") != std::string::npos) {
                current.ip = extract_attr(line, "addr");
                current.id = convert_device_id(current.ip);
            }
            if (line.find("addrtype=\"mac\"") != std::string::npos) {
                current.mac_address    = extract_attr(line, "addr");
                current.vendor = extract_attr(line, "vendor");
            }
        }

        if (line.find("<hostname ") != std::string::npos) {
            if (current.name.empty())
                current.name = extract_attr(line, "name");
        }
    }

    return devices;
}

std::vector<OpenPort> NetworkScanner::parse_ports_xml(const std::string& xml){
    std::vector<OpenPort> ports;
    std::istringstream stream(xml);
    std::string line;
    bool in_port = false;
    OpenPort current;

    while (std::getline(stream, line)) {
        if (line.find("<port ") != std::string::npos) {
            in_port = true;
            current = OpenPort{};
            current.protocol = extract_attr(line, "protocol");
            std::string pid  = extract_attr(line, "portid");
            if (!pid.empty()) current.port_number = std::stoi(pid);
        }
        if (line.find("</port>") != std::string::npos) {
            if (in_port && current.port_number > 0)
                ports.push_back(current);
            in_port = false;
        }
        if (!in_port) continue;

        if (line.find("<state") != std::string::npos)
            if (line.find("state=\"open\"") == std::string::npos)
                in_port = false;

        if (line.find("<service") != std::string::npos)
            current.service = extract_attr(line, "name");
    }
    return ports;
}

NetworkInfo NetworkScanner::get_local_network(){
     NetworkInfo info;

    std::string routes = exec_cmd("ip route 2>/dev/null");
    std::regex  dev_re(R"(default via\s+(\S+)\s+dev\s+(\S+))");
    std::smatch m;

    if (!std::regex_search(routes, m, dev_re))
        return info; 

    info.gateway        = m[1];
    info.interface_name = m[2];

    std::string addr_out = exec_cmd(
        "ip -4 addr show " + info.interface_name + " 2>/dev/null"
    );
    std::regex inet_re(R"(inet\s+(\d+\.\d+\.\d+\.\d+)/(\d+))");
    if (!std::regex_search(addr_out, m, inet_re))
        return info;

    info.local_ip       = m[1];
    int prefix          = std::stoi(m[2]);
    info.subnet_cidr    = compute_subnet_cidr(info.local_ip, prefix);
    info.is_valid       = true;

    return info;
}

std::string NetworkScanner::compute_subnet_cidr(
    const std::string& ip, int prefix)
{
    struct in_addr addr;
    inet_pton(AF_INET, ip.c_str(), &addr);
    uint32_t ip_int  = ntohl(addr.s_addr);
    uint32_t mask    = prefix > 0 ? (~0u << (32 - prefix)) : 0;
    uint32_t network = ip_int & mask;

    struct in_addr net_addr;
    net_addr.s_addr = htonl(network);
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &net_addr, buf, sizeof(buf));

    return std::string(buf) + "/" + std::to_string(prefix);
}

std::vector<Device> NetworkScanner::scan_targets(const std::string& subnet){
    std::string cmd = "nmap -sn " + subnet + " -oX -";

    std::string xml = exec_cmd(cmd);

    return parse_hosts_xml(xml);
}

std::vector<OpenPort> NetworkScanner::scan_ports(const std::string& target_ip, const std::string& port_range){
    std::string cmd = "nmap -sV -T4 --open "
                    + target_ip
                    + " -p " + port_range
                    + " -oX -";

    std::string xml = exec_cmd(cmd);

    return parse_ports_xml(xml);
}