#include "matching_engine.h"
#include <iostream>
#include <iomanip> // For std::put_time
#include <ctime>   // For std::localtime
#define _CRT_SECURE_NO_WARNINGS


void MatchingEngine::processOrder(const Order& order) {
    auto trades = orderBook.matchOrder(order);
    if (!trades.empty()) {
        for (const auto& trade : trades) {
            std::time_t t = std::chrono::system_clock::to_time_t(trade.timestamp);
            std::tm buf;
            localtime_s(&buf, &t);
            std::cout << std::put_time(&buf, "%F %T");

            std::cout << "[TRADE] ID: " << trade.tradeId
                      << ", BuyID: " << trade.buyOrderId
                      << ", SellID: " << trade.sellOrderId
                      << ", Price: " << trade.price
                      << ", Qty: " << trade.quantity
                      << " at " << std::put_time(std::localtime(&t), "%F %T")
                      << std::endl;
        }
    } else {
        std::cout << "Order added to book: Order ID " << order.id << std::endl;
    }
}

void MatchingEngine::printOrderBook() const {
    orderBook.printOrderBook();
}
