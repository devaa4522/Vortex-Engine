#pragma once
#include <string>
#include <vector>
#include <chrono>
#include <nlohmann/json.hpp>
#include "vortex/Utils.h" // Include after json.hpp

enum class OrderSide { Buy, Sell };
enum class OrderType { Limit, Market, Stop, Iceberg, FillOrKill, ImmediateOrCancel };
enum class OrderStatus { Active, Filled, Cancelled, Expired, Pending };

NLOHMANN_JSON_SERIALIZE_ENUM(OrderSide, {
    {OrderSide::Buy, "buy"}, {OrderSide::Sell, "sell"}
})
NLOHMANN_JSON_SERIALIZE_ENUM(OrderType, {
    {OrderType::Limit, "limit"}, {OrderType::Market, "market"}, {OrderType::Stop, "stop"},
    {OrderType::Iceberg, "iceberg"}, {OrderType::FillOrKill, "fok"}, {OrderType::ImmediateOrCancel, "ioc"}
})
NLOHMANN_JSON_SERIALIZE_ENUM(OrderStatus, {
    {OrderStatus::Active, "active"}, {OrderStatus::Filled, "filled"}, {OrderStatus::Cancelled, "cancelled"},
    {OrderStatus::Expired, "expired"}, {OrderStatus::Pending, "pending"}
})

struct Order {
    uint64_t id;
    OrderSide side;
    OrderType type;
    double price;
    double stopPrice;
    uint64_t quantity;
    uint64_t remaining;
    uint64_t peakSize;
    uint64_t visibleQuantity;
    std::chrono::system_clock::time_point timestamp;
    std::chrono::system_clock::time_point expiry;
    OrderStatus status;
    std::vector<std::string> auditTrail;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Order, id, side, type, price, stopPrice, quantity, remaining, peakSize, visibleQuantity, timestamp, expiry, status, auditTrail)