#pragma once
#include <string>
#include <chrono>

struct Order {
    int id;
    std::string side; // "buy" or "sell"
    std::string type; // "limit" or "market"
    double price;
    int quantity;
    std::chrono::high_resolution_clock::time_point timestamp;
};
