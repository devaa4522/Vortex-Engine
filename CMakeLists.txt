cmake_minimum_required(VERSION 3.20)
project(vortex LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ───────── Core library ─────────
add_library(vortex_core
    src/Order.cpp
    src/Trade.cpp
    src/OrderBook.cpp
    src/Utils.cpp
    src/matching_engine.cpp
)
target_include_directories(vortex_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

# Find and link nlohmann_json JUST for the core library
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(vortex_core PUBLIC nlohmann_json::nlohmann_json)


# ───────── Executables ─────────
# --- CLI ---
add_executable(vortex src/main.cpp)
target_link_libraries(vortex PRIVATE vortex_core)

# --- API Server ---
add_executable(vortex_api_server src/api_server.cpp)

# Find and link Crow JUST for the api server
find_package(Crow CONFIG REQUIRED)
target_link_libraries(vortex_api_server PRIVATE vortex_core Crow::Crow)
# Add this definition to silence the _WIN32_WINNT warning
target_compile_definitions(vortex_api_server PRIVATE _WIN32_WINNT=0x0A00)