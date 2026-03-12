import requests

SERVER_URL = "http://localhost:8080"

def get_network_info() -> dict:
    r = requests.get(f"{SERVER_URL}/api/network", timeout=5)
    r.raise_for_status()
    return r.json()

def scan_network(subnet: str = "") -> dict:
    """Returneaza { subnet, total_found, devices: [{ip, mac, vendor}] }"""
    r = requests.post(f"{SERVER_URL}/api/scan",
                      json={"subnet": subnet} if subnet else {},
                      timeout=60)
    r.raise_for_status()
    return r.json()

def get_device(device_id: str) -> dict:
    """Returneaza device-ul complet cu porturi (daca sunt deja scanate)"""
    r = requests.get(f"{SERVER_URL}/api/devices/{device_id}", timeout=5)
    r.raise_for_status()
    return r.json()

# TODO: port scan — de implementat dupa ce server-ul expune /api/devices/{id}/scan-status
def get_scan_status(device_id: str) -> bool:
    raise NotImplementedError("Port scan not yet implemented")

def set_target(device_id: str) -> dict:
    r = requests.post(f"{SERVER_URL}/api/devices/target",
                      json={"device_id": device_id},
                      timeout=5)
    r.raise_for_status()
    return r.json()

def clear_target():
    r = requests.delete(f"{SERVER_URL}/api/devices/target", timeout=5)
    r.raise_for_status()