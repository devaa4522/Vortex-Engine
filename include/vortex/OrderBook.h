#pragma once
#include "Order.h"
#include "Trade.h"
#include <vector>
#include <deque>
#include <map>
#include <functional> // For std::greater

class OrderBook {
public:
    OrderBook();

    uint64_t addOrder(Order order);
    bool modifyOrder(uint64_t orderId, double newPrice, uint64_t newQuantity);
    bool cancelOrder(uint64_t orderId);
    
    void expireOrders();
    void triggerStopOrders(double lastTradePrice);
    void printOrderBook() const;
    void printTradeHistory() const;
    
    // Persistence
    void save(const std::string& filename) const;
    void load(const std::string& filename);

    // Public accessors for engine
    const std::map<uint64_t, Order>& getAllOrders() const { return allOrders; }
    const std::vector<Trade>& getTrades() const { return trades; }
    const std::map<double, std::deque<Order>, std::greater<double>>& getBuyOrders() const { return buyOrders; }
    const std::map<double, std::deque<Order>>& getSellOrders() const { return sellOrders; }


private:
    // Data Structures: Use maps for price-time priority.
    // Buys: sorted high to low price. Sells: sorted low to high price.
    std::map<double, std::deque<Order>, std::greater<double>> buyOrders;
    std::map<double, std::deque<Order>> sellOrders;

    std::map<uint64_t, Order> allOrders;
    std::vector<Order> stopOrders;
    std::vector<Trade> trades;
    
    uint64_t nextOrderId;
    uint64_t nextTradeId;

    void matchOrders();
    void matchAdvancedOrder(Order& order);
    void addTrade(const Trade& trade);
    void addOrderToBook(Order order);
    void removeOrderFromBook(uint64_t orderId);
    void replenishIcebergOrder(Order& order);
    void addAuditTrail(Order& order, const std::string& action);
};