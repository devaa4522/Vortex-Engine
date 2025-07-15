# Vortex Engine: A High-Performance C++ Matching Engine

Vortex is a high-performance financial matching engine written in modern C++. It features a robust, multithreaded architecture designed for high throughput and low-latency order processing. The engine supports a variety of advanced order types and provides a persistent storage mechanism, a command-line interface (CLI) for direct interaction, and a REST/WebSocket API for programmatic trading.

## ✨ Key Features

* **High-Performance Core**: Utilizes efficient, price-sorted data structures (`std::map`) for O(1) access to the best bid and ask, ensuring rapid order matching.
* **Multithreaded Architecture**: Employs a producer-consumer model with a thread-safe work queue. API threads act as producers, instantly accepting requests, while a dedicated engine thread acts as a consumer, ensuring safe and sequential order processing without race conditions.
* **Advanced Order Types**: In addition to standard `Limit` and `Market` orders, the engine supports:
    * `Immediate-Or-Cancel (IOC)`
    * `Fill-Or-Kill (FOK)`
    * `Stop` Orders
    * `Iceberg` Orders
* **Persistent Storage**: Order book state and trade history are saved to a robust JSON file, allowing the engine's state to be restored after a restart.
* **Dual Interfaces**:
    * **Interactive CLI**: A command-line tool for manually adding/canceling orders, viewing the book, and checking trade history.
    * **RESTful API Server**: A multithreaded server built with Crow for programmatic trading and querying engine state.
* **Real-Time Updates**: A WebSocket endpoint provides real-time snapshots of the order book and trade history.

## 🛠️ Build Instructions

### Prerequisites

* A C++17 compliant compiler (e.g., MSVC, GCC, Clang)
* [CMake](https://cmake.org/download/) (version 3.20+)
* [vcpkg](https://github.com/microsoft/vcpkg) package manager

### Dependencies

The project requires the following libraries, which can be installed via vcpkg:

```sh
vcpkg install crow:x64-windows nlohmann-json:x64-windows
```
*(Use the appropriate triplet for your platform, e.g., `:x64-linux` or `:x64-osx`)*

### Compiling

1.  **Configure with CMake:**
    From the project root, run CMake to generate the build files. Make sure to point it to your vcpkg toolchain file.

    ```sh
    # Replace <path-to-vcpkg> with your actual vcpkg installation path
    cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake
    ```

2.  **Build the Project:**
    Use CMake's build command to compile the executables.

    ```sh
    cmake --build build --config Release
    ```

This will create two executables in the `build/Release/` directory: `vortex.exe` and `vortex_api_server.exe`.

## 🚀 Usage

### Command-Line Interface (CLI)

The CLI provides direct, blocking access to the matching engine. It's useful for testing and manual interaction.

```sh
# Run the CLI
./build/Release/vortex.exe

# Example Commands
> add buy limit 100.50 10
> book
> add sell limit 100.50 5
> trades
> cancel 1
```

### API Server

The API server provides a high-performance, non-blocking interface for programmatic trading.

1.  **Run the Server:**
    ```sh
    ./build/Release/vortex_api_server.exe
    ```
    The server will start on `http://localhost:8080`.

2.  **API Endpoints:**

    * `POST /api/v1/orders`
        * Submits a new order. The server responds immediately with `202 Accepted` and processes the order in the background.
        * **Body:**
            ```json
            {
                "side": "buy",
                "type": "limit",
                "quantity": 10,
                "price": 150.75,
                "peakSize": 0,    // Optional, for iceberg orders
                "stopPrice": 0,   // Optional, for stop orders
                "expirySec": 0    // Optional, time in seconds
            }
            ```

    * `GET /api/v1/orderbook`
        * Returns a snapshot of the current order book.

    * `GET /api/v1/trades`
        * Returns a list of all trades executed.

    * `GET /api/v1/orders/<uint64_t>`
        * Returns the details of a specific order by its ID.

    * `WS /api/v1/ws`
        * WebSocket endpoint that broadcasts a full snapshot of the order book and trades every second.