#include "order_book.h"
#include "trade.h"
#include <iostream>
#include <chrono>
#include <iomanip> // For std::put_time

static int tradeCounter = 1; // At the top of order_book.cpp

void OrderBook::addOrder(const Order& order) {
    if (order.side == "buy") {
        buyOrders[order.price].push(order);
    } else {
        sellOrders[order.price].push(order);
    }
}

// Updated: Now returns std::vector<Trade> and generates detailed trade records
std::vector<Trade> OrderBook::matchOrder(const Order& order) {
    std::vector<Trade> trades; // Changed from std::vector<Order> to std::vector<Trade>
    int qty = order.quantity;

    if (order.type == "market") {
        if (order.side == "buy") {
            // Buy market order matches against sellOrders (ascending)
            auto it = sellOrders.begin();
            while (it != sellOrders.end() && qty > 0) {
                while (!it->second.empty() && qty > 0) {
                    Order top = it->second.front();
                    it->second.pop();
                    int tradeQty = std::min(qty, top.quantity);
                    qty -= tradeQty;

                    // Create Trade record
                    Trade trade;
                    trade.tradeId = tradeCounter++;
                    trade.buyOrderId = order.id;
                    trade.sellOrderId = top.id;
                    trade.price = it->first;
                    trade.quantity = tradeQty;
                    trade.timestamp = std::chrono::system_clock::now();
                    trades.push_back(trade);

                    if (top.quantity > tradeQty) {
                        top.quantity -= tradeQty;
                        it->second.push(top);
                    }
                }
                if (it->second.empty()) it = sellOrders.erase(it);
                else ++it;
            }
        } else {
            // Sell market order matches against buyOrders (descending)
            auto it = buyOrders.begin();
            while (it != buyOrders.end() && qty > 0) {
                while (!it->second.empty() && qty > 0) {
                    Order top = it->second.front();
                    it->second.pop();
                    int tradeQty = std::min(qty, top.quantity);
                    qty -= tradeQty;

                    // Create Trade record
                    Trade trade;
                    trade.tradeId = tradeCounter++;
                    trade.buyOrderId = top.id;
                    trade.sellOrderId = order.id;
                    trade.price = it->first;
                    trade.quantity = tradeQty;
                    trade.timestamp = std::chrono::system_clock::now();
                    trades.push_back(trade);

                    if (top.quantity > tradeQty) {
                        top.quantity -= tradeQty;
                        it->second.push(top);
                    }
                }
                if (it->second.empty()) it = buyOrders.erase(it);
                else ++it;
            }
        }
    } else {
        if (order.side == "buy") {
            // Buy limit order matches against sellOrders
            auto it = sellOrders.begin();
            while (it != sellOrders.end() && qty > 0) {
                if (order.price < it->first) break;
                while (!it->second.empty() && qty > 0) {
                    Order top = it->second.front();
                    it->second.pop();
                    int tradeQty = std::min(qty, top.quantity);
                    qty -= tradeQty;

                    // Create Trade record
                    Trade trade;
                    trade.tradeId = tradeCounter++;
                    trade.buyOrderId = order.id;
                    trade.sellOrderId = top.id;
                    trade.price = it->first;
                    trade.quantity = tradeQty;
                    trade.timestamp = std::chrono::system_clock::now();
                    trades.push_back(trade);

                    if (top.quantity > tradeQty) {
                        top.quantity -= tradeQty;
                        it->second.push(top);
                    }
                }
                if (it->second.empty()) it = sellOrders.erase(it);
                else ++it;
            }
            if (qty > 0) {
                Order remaining = order;
                remaining.quantity = qty;
                addOrder(remaining);
            }
        } else {
            // Sell limit order matches against buyOrders
            auto it = buyOrders.begin();
            while (it != buyOrders.end() && qty > 0) {
                if (order.price > it->first) break;
                while (!it->second.empty() && qty > 0) {
                    Order top = it->second.front();
                    it->second.pop();
                    int tradeQty = std::min(qty, top.quantity);
                    qty -= tradeQty;

                    // Create Trade record
                    Trade trade;
                    trade.tradeId = tradeCounter++;
                    trade.buyOrderId = top.id;
                    trade.sellOrderId = order.id;
                    trade.price = it->first;
                    trade.quantity = tradeQty;
                    trade.timestamp = std::chrono::system_clock::now();
                    trades.push_back(trade);

                    if (top.quantity > tradeQty) {
                        top.quantity -= tradeQty;
                        it->second.push(top);
                    }
                }
                if (it->second.empty()) it = buyOrders.erase(it);
                else ++it;
            }
            if (qty > 0) {
                Order remaining = order;
                remaining.quantity = qty;
                addOrder(remaining);
            }
        }
    }
    return trades;
}

void OrderBook::printOrderBook() const {
    std::cout << "\n--- ORDER BOOK ---\n";
    std::cout << "ASKS (SELL):\n";
    // Sell orders: lowest price first
    for (const auto& [price, queue] : sellOrders) {
        int totalQty = 0;
        std::queue<Order> temp = queue;
        while (!temp.empty()) {
            totalQty += temp.front().quantity;
            temp.pop();
        }
        std::cout << "Price: " << price << " | Qty: " << totalQty << "\n";
    }
    std::cout << "------------------\n";
    std::cout << "BIDS (BUY):\n";
    // Buy orders: highest price first
    for (const auto& [price, queue] : buyOrders) {
        int totalQty = 0;
        std::queue<Order> temp = queue;
        while (!temp.empty()) {
            totalQty += temp.front().quantity;
            temp.pop();
        }
        std::cout << "Price: " << price << " | Qty: " << totalQty << "\n";
    }
    std::cout << "------------------\n";
}


// void OrderBook::printOrderBook() const {
//     std::cout << "\nOrder Book:\n";
//     std:: cout << "--------------------------\n";
//     std::cout << "Buy Orders:\n";

//     for (const auto& [price, queue] : buyOrders) {
//         std::cout << "  Price: " << price << " | Orders: " << queue.size() << "\n";
//     }
//     std:: cout << "--------------------------\n";
//     std::cout << "Sell Orders:\n";
//     for (const auto& [price, queue] : sellOrders) {
//         std::cout << "  Price: " << price << " | Orders: " << queue.size() << "\n";
//     }
//     std:: cout << "--------------------------\n";
//     std::cout << std::endl;
// }
