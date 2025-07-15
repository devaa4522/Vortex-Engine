#include "vortex/matching_engine.h"
#include "vortex/Utils.h"
#include <thread>

MatchingEngine::MatchingEngine() {}

// --- High-Performance API Methods ---

void MatchingEngine::postOrder(OrderSide side, OrderType type, double price, double stopPrice,
                                 uint64_t quantity, uint64_t peakSize, uint64_t expirySec) {
    OrderCommand cmd = {side, type, price, stopPrice, quantity, peakSize, expirySec};
    workQueue.push(cmd);
}

void MatchingEngine::run() {
    std::thread([this]() {
        while (true) {
            OrderCommand cmd;
            workQueue.wait_and_pop(cmd);
            processOrder(cmd);
        }
    }).detach();
}

void MatchingEngine::processOrder(const OrderCommand& cmd) {
    std::lock_guard<std::mutex> lock(engine_mutex);
    Order order;
    order.side = cmd.side;
    order.type = cmd.type;
    order.price = cmd.price;
    order.stopPrice = cmd.stopPrice;
    order.quantity = cmd.quantity;
    order.peakSize = cmd.peakSize;
    if (cmd.expirySec > 0) {
        order.expiry = Utils::now() + std::chrono::seconds(cmd.expirySec);
    } else {
        order.expiry = std::chrono::system_clock::time_point::min();
    }
    orderBook.addOrder(std::move(order));
}


// --- CLI Tool Methods ---

uint64_t MatchingEngine::addOrder(OrderSide side, OrderType type, double price, double stopPrice, uint64_t quantity, uint64_t peakSize, uint64_t expirySec) {
    std::lock_guard<std::mutex> lock(engine_mutex);
    Order order;
    order.side = side;
    order.type = type;
    order.price = price;
    order.stopPrice = stopPrice;
    order.quantity = quantity;
    order.peakSize = peakSize;
     if (expirySec > 0) {
        order.expiry = Utils::now() + std::chrono::seconds(expirySec);
    } else {
        order.expiry = std::chrono::system_clock::time_point::min();
    }
    return orderBook.addOrder(std::move(order));
}

bool MatchingEngine::cancelOrder(uint64_t orderId) {
    std::lock_guard<std::mutex> lock(engine_mutex);
    return orderBook.cancelOrder(orderId);
}

bool MatchingEngine::modifyOrder(uint64_t orderId, double newPrice, uint64_t newQuantity) {
    std::lock_guard<std::mutex> lock(engine_mutex);
    return orderBook.modifyOrder(orderId, newPrice, newQuantity);
}


// --- Common Query Methods ---

void MatchingEngine::printOrderBook() const {
    std::lock_guard<std::mutex> lock(engine_mutex);
    orderBook.printOrderBook();
}

void MatchingEngine::printTradeHistory() const {
    std::lock_guard<std::mutex> lock(engine_mutex);
    orderBook.printTradeHistory();
}

void MatchingEngine::save(const std::string& filename) const {
    std::lock_guard<std::mutex> lock(engine_mutex);
    orderBook.save(filename);
}

void MatchingEngine::load(const std::string& filename) {
    std::lock_guard<std::mutex> lock(engine_mutex);
    orderBook.load(filename);
}

std::optional<Order> MatchingEngine::getOrderById(uint64_t orderId) const {
    std::lock_guard<std::mutex> lock(engine_mutex);
    const auto& allOrders = orderBook.getAllOrders();
    auto it = allOrders.find(orderId);
    if (it != allOrders.end()) {
        return it->second;
    }
    return std::nullopt;
}

nlohmann::json MatchingEngine::getOrderBookSnapshot() const {
    std::lock_guard<std::mutex> lock(engine_mutex);
    nlohmann::json j;
    j["buy"] = nlohmann::json::array();
    j["sell"] = nlohmann::json::array();
    for (const auto& [price, level] : orderBook.getBuyOrders()) {
        for (const auto& o : level) j["buy"].push_back(o);
    }
     for (const auto& [price, level] : orderBook.getSellOrders()) {
        for (const auto& o : level) j["sell"].push_back(o);
    }
    return j;
}

nlohmann::json MatchingEngine::getTradeHistory() const {
    std::lock_guard<std::mutex> lock(engine_mutex);
    return nlohmann::json(orderBook.getTrades());
}