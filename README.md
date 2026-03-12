# Network Device Manager

This project is a client-server application designed to manage devices on a local network. It consists of a C++ backend server and a Python-based graphical user interface (GUI) client.

## Architecture

-   **Server**: A C++ application that exposes a RESTful API for device management and network scanning.
-   **Client**: A Python application with a PyQt6 GUI that communicates with the server to provide a user-friendly interface for managing devices.

---

## Server (C++)

The server is a lightweight, high-performance C++ application that forms the backend of the Network Device Manager.

### Features

-   **REST API**: Exposes endpoints for device management (e.g., add, remove, get device information) and for initiating network scans.
-   **Network Scanning**: Capable of scanning the local network to discover connected devices.
-   **Dependencies**:
    -   [httplib](https://github.com/yhirose/cpp-httplib) for handling HTTP requests.
    -   [nlohmann/json](https://github.com/nlohmann/json) for JSON serialization and deserialization.

### Building and Running the Server

1.  **Prerequisites**:
    -   A C++17 compatible compiler (e.g., GCC, Clang)
    -   CMake 3.10+

2.  **Build Steps**:
    ```bash
    mkdir -p build
    cd build
    cmake ..
    make
    ```

3.  **Running the Server**:
    ```bash
    ./<executable_name> # The executable is typically found in the build directory
    ```
    *(Note: The exact executable name may vary based on the CMake configuration)*

---

## Client (Python)

The client is a desktop GUI application built with Python and the PyQt6 framework.

### Features

-   **User-Friendly Interface**: Provides an intuitive GUI to interact with the server.
-   **Device Management**: Allows users to view, add, and manage devices discovered on the network.
-   **Real-time Updates**: Communicates with the server to get real-time information about network devices.

### Setting up and Running the Client

1.  **Prerequisites**:
    -   Python 3.6+
    -   pip

2.  **Installation**:
    Navigate to the `Client` directory and install the required dependencies:
    ```bash
    pip install -r requirements.txt
    ```

3.  **Running the Client**:
    ```bash
    python Client/src/main.py
    ```

---

## Project Structure

```
├── Client/
│   ├── requirements.txt
│   └── src/
│       ├── api.py           # Handles communication with the server's API
│       ├── main_window.py   # Defines the main GUI window
│       └── main.py          # Entry point for the client application
└── Server/
    ├── CMakeLists.txt
    ├── main.cpp             # Entry point for the server application
    ├── external/            # Third-party libraries
    └── src/
        ├── api/             # API route handlers
        ├── domain/          # Core business logic and data models
        ├── scanner/         # Network scanning functionality
        └── services/        # Services that orchestrate domain logic
```
