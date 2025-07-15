#pragma once
#include "OrderBook.h"
#include "ThreadSafeQueue.h"
#include <string>
#include <optional>
#include <functional>
#include <nlohmann/json.hpp>
#include <mutex>

struct OrderCommand {
    OrderSide side;
    OrderType type;
    double price;
    double stopPrice;
    uint64_t quantity;
    uint64_t peakSize;
    uint64_t expirySec;
};

class MatchingEngine {
public:
    MatchingEngine();

    // --- Methods for the High-Performance API Server ---
    void postOrder(OrderSide side, OrderType type, double price, double stopPrice, uint64_t quantity, uint64_t peakSize, uint64_t expirySec);
    void run();

    // --- Methods for the CLI Tool ---
    // We add these back for direct, blocking access for the CLI.
    uint64_t addOrder(OrderSide side, OrderType type, double price, double stopPrice, uint64_t quantity, uint64_t peakSize, uint64_t expirySec);
    bool cancelOrder(uint64_t orderId);
    bool modifyOrder(uint64_t orderId, double newPrice, uint64_t newQuantity);

    // --- Common Query Methods (Thread-Safe) ---
    void printOrderBook() const;
    void printTradeHistory() const;
    void save(const std::string& filename) const;
    void load(const std::string& filename);
    std::optional<Order> getOrderById(uint64_t orderId) const;
    nlohmann::json getOrderBookSnapshot() const;
    nlohmann::json getTradeHistory() const;

private:
    void processOrder(const OrderCommand& cmd);

    OrderBook orderBook;
    mutable std::mutex engine_mutex;
    ThreadSafeQueue<OrderCommand> workQueue;
};