import requests

SERVER_URL = "http://localhost:5000"


def scan_network() -> list[dict]:
    """
    Request a network scan from the server.
    Returns a list of device dicts, e.g.:
        [{"ip": "192.168.1.1", "mac": "aa:bb:cc:dd:ee:ff", "hostname": "router"}, ...]
    Raises requests.RequestException on failure.
    """
    response = requests.get(f"{SERVER_URL}/scan", timeout=10)
    response.raise_for_status()
    return response.json()
