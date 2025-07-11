#pragma once
#include "order.h"
#include "trade.h"
#include <map>
#include <queue>
#include <vector>

class OrderBook {
public:
    void addOrder(const Order& order);
    // std::vector<Order> matchOrder(const Order& order);
    std::vector<Trade> matchOrder(const Order& order);
    void printOrderBook() const;
private:
    // Buy orders: highest price first
    std::map<double, std::queue<Order>, std::greater<double>> buyOrders;
    // Sell orders: lowest price first
    std::map<double, std::queue<Order>> sellOrders;
};
