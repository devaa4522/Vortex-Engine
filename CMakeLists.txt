cmake_minimum_required(VERSION 3.10)
project(MatchingEngine)
set(CMAKE_CXX_STANDARD 17)
add_executable(matching_engine
    src/main.cpp
    src/order_book.cpp
    src/matching_engine.cpp
)
