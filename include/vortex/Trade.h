#pragma once
#include <string>
#include <chrono>
#include <nlohmann/json.hpp>
#include "vortex/Utils.h" // Must be included for the time_point serializer

struct Trade {
    uint64_t tradeId;
    uint64_t buyOrderId;
    uint64_t sellOrderId;
    double price;
    uint64_t quantity;
    std::chrono::system_clock::time_point timestamp;
};

// Use this macro to explicitly define serialization for the Trade struct
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Trade, tradeId, buyOrderId, sellOrderId, price, quantity, timestamp)