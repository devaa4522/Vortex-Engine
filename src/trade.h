#pragma once
#include <chrono>

struct Trade {
    int tradeId;
    int buyOrderId;
    int sellOrderId;
    double price;
    int quantity;
    std::chrono::system_clock::time_point timestamp;
};
