#pragma once
#include "order_book.h"

class MatchingEngine {
public:
    void processOrder(const Order& order);
    void printOrderBook() const;
private:
    OrderBook orderBook;
};
