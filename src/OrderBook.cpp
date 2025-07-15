#include "vortex/OrderBook.h"
#include "vortex/Utils.h"
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <iostream>

using json = nlohmann::json;

OrderBook::OrderBook() : nextOrderId(1), nextTradeId(1) {}

uint64_t OrderBook::addOrder(Order order) {
    order.id = nextOrderId++;
    order.timestamp = Utils::now();
    order.remaining = order.quantity;
    order.status = OrderStatus::Active;
    if (order.type == OrderType::Iceberg) {
        order.visibleQuantity = std::min(order.quantity, order.peakSize);
    } else {
        order.visibleQuantity = order.quantity;
    }
    addAuditTrail(order, "Order received");
    allOrders[order.id] = order;
    if (order.type == OrderType::Stop) {
        order.status = OrderStatus::Pending;
        stopOrders.push_back(order);
        allOrders[order.id] = order;
        addAuditTrail(allOrders[order.id], "Order pending (stop)");
        return order.id;
    }
    if (order.type == OrderType::FillOrKill || order.type == OrderType::ImmediateOrCancel) {
        matchAdvancedOrder(allOrders.at(order.id));
    } else {
        addOrderToBook(order);
        matchOrders();
    }
    return order.id;
}

void OrderBook::addOrderToBook(Order order) {
    addAuditTrail(allOrders.at(order.id), "Order added to book");
    if (order.side == OrderSide::Buy) {
        buyOrders[order.price].push_back(order);
    } else {
        sellOrders[order.price].push_back(order);
    }
}

void OrderBook::matchOrders() {
    while (!buyOrders.empty() && !sellOrders.empty()) {
        auto& bestBuyLevel = buyOrders.begin()->second;
        auto& bestSellLevel = sellOrders.begin()->second;
        if (bestBuyLevel.front().price >= bestSellLevel.front().price) {
            Order& buy = bestBuyLevel.front();
            Order& sell = bestSellLevel.front();
            uint64_t matchedQty = std::min(buy.remaining, sell.remaining);
            double tradePrice = sell.price;
            Trade trade{nextTradeId++, buy.id, sell.id, tradePrice, matchedQty, Utils::now()};
            addTrade(trade);
            buy.remaining -= matchedQty;
            sell.remaining -= matchedQty;
            allOrders.at(buy.id).remaining = buy.remaining;
            allOrders.at(sell.id).remaining = sell.remaining;
            if (buy.remaining == 0) {
                addAuditTrail(allOrders.at(buy.id), "Order fully filled");
                allOrders.at(buy.id).status = OrderStatus::Filled;
                bestBuyLevel.pop_front();
            } else {
                addAuditTrail(allOrders.at(buy.id), "Order partially filled");
            }
            if (sell.remaining == 0) {
                addAuditTrail(allOrders.at(sell.id), "Order fully filled");
                allOrders.at(sell.id).status = OrderStatus::Filled;
                bestSellLevel.pop_front();
            } else {
                 addAuditTrail(allOrders.at(sell.id), "Order partially filled");
            }
            if (bestBuyLevel.empty()) buyOrders.erase(buyOrders.begin());
            if (bestSellLevel.empty()) sellOrders.erase(sellOrders.begin());
        } else {
            break;
        }
    }
}

void OrderBook::matchAdvancedOrder(Order& order) {
    if (order.type == OrderType::FillOrKill) {
        uint64_t fillable = 0;
        if (order.side == OrderSide::Buy) {
            for (auto const& [price, level] : sellOrders) {
                 if (price <= order.price) { for(const auto& o : level) fillable += o.remaining; }
            }
        } else {
             for (auto const& [price, level] : buyOrders) {
                 if (price >= order.price) { for(const auto& o : level) fillable += o.remaining; }
            }
        }
        if (fillable < order.quantity) {
            order.status = OrderStatus::Cancelled;
            addAuditTrail(order, "FOK Cancelled: insufficient liquidity");
            allOrders[order.id] = order;
            return;
        }
    }
    uint64_t qtyToFill = order.quantity;
    if (order.side == OrderSide::Buy) {
        for (auto it = sellOrders.begin(); it != sellOrders.end() && qtyToFill > 0; ) {
            auto& level = it->second;
            for(auto orderIt = level.begin(); orderIt != level.end() && qtyToFill > 0; ) {
                uint64_t matchedQty = std::min(qtyToFill, orderIt->remaining);
                Trade trade{nextTradeId++, order.id, orderIt->id, orderIt->price, matchedQty, Utils::now()};
                addTrade(trade);
                qtyToFill -= matchedQty;
                orderIt->remaining -= matchedQty;
                allOrders.at(orderIt->id).remaining = orderIt->remaining;
                if (orderIt->remaining == 0) {
                    allOrders.at(orderIt->id).status = OrderStatus::Filled;
                    addAuditTrail(allOrders.at(orderIt->id), "Filled by IOC/FOK order");
                    orderIt = level.erase(orderIt);
                } else {
                    ++orderIt;
                }
            }
            if (level.empty()) it = sellOrders.erase(it); else ++it;
        }
    }
    order.remaining -= (order.quantity - qtyToFill);
    if (order.remaining == 0) {
        order.status = OrderStatus::Filled;
        addAuditTrail(order, "Order fully filled (IOC/FOK)");
    } else {
        order.status = OrderStatus::Cancelled;
        addAuditTrail(order, "Remaining part of order cancelled (IOC/FOK)");
    }
    allOrders[order.id] = order;
}

bool OrderBook::modifyOrder(uint64_t orderId, double newPrice, uint64_t newQuantity) {
    auto it = allOrders.find(orderId);
    if (it == allOrders.end() || it->second.status != OrderStatus::Active) return false;
    Order originalOrder = it->second;
    cancelOrder(orderId);
    Order newOrder = originalOrder;
    newOrder.price = newPrice;
    newOrder.quantity = newQuantity;
    newOrder.remaining = newQuantity;
    newOrder.timestamp = Utils::now();
    newOrder.status = OrderStatus::Active;
    addAuditTrail(newOrder, "Order modified");
    allOrders[orderId] = newOrder;
    addOrderToBook(newOrder);
    matchOrders();
    return true;
}

bool OrderBook::cancelOrder(uint64_t orderId) {
    auto it = allOrders.find(orderId);
    if (it == allOrders.end() || (it->second.status != OrderStatus::Active && it->second.status != OrderStatus::Pending)) return false;
    it->second.status = OrderStatus::Cancelled;
    addAuditTrail(it->second, "Order cancelled");
    removeOrderFromBook(orderId);
    return true;
}

void OrderBook::removeOrderFromBook(uint64_t orderId) {
    auto it = allOrders.find(orderId);
    if (it == allOrders.end()) return;
    const Order& order = it->second;
    auto removeFromBook = [&](auto& book) {
        auto level_it = book.find(order.price);
        if (level_it != book.end()) {
            auto& level = level_it->second;
            level.erase(std::remove_if(level.begin(), level.end(), [orderId](const Order& o) { return o.id == orderId; }), level.end());
            if (level.empty()) book.erase(level_it);
        }
    };
    if (order.side == OrderSide::Buy) removeFromBook(buyOrders);
    else removeFromBook(sellOrders);
}

void OrderBook::addAuditTrail(Order& order, const std::string& action) {
    order.auditTrail.push_back(action + " @ " + Utils::formatTime(Utils::now()));
}

void OrderBook::addTrade(const Trade& trade) {
    trades.push_back(trade);
    std::cout << "[TRADE EXECUTED] ID: " << trade.tradeId << ", Price: " << trade.price << ", Qty: " << trade.quantity << std::endl;
}

void OrderBook::save(const std::string& filename) const {
    std::ofstream ofs(filename);
    json j;
    j["orders"] = allOrders;
    j["trades"] = trades;
    j["nextOrderId"] = nextOrderId;
    j["nextTradeId"] = nextTradeId;
    ofs << j.dump(4);
}

void OrderBook::load(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }
    json j;
    ifs >> j;
    allOrders.clear();
    buyOrders.clear();
    sellOrders.clear();
    trades.clear();
    stopOrders.clear();
    nextOrderId = j.at("nextOrderId").get<uint64_t>();
    nextTradeId = j.at("nextTradeId").get<uint64_t>();
    j.at("orders").get_to(allOrders);
    j.at("trades").get_to(trades);
    for(const auto& [id, order] : allOrders) {
         if (order.status == OrderStatus::Active) addOrderToBook(order);
         else if (order.status == OrderStatus::Pending && order.type == OrderType::Stop) stopOrders.push_back(order);
    }
}

void OrderBook::printOrderBook() const {
     auto print_table = [](const std::string& title, const auto& book) {
        std::cout << title << ":\n" << std::left
            << std::setw(5) << "ID" << std::setw(10) << "Price" << std::setw(8) << "Qty"
            << std::setw(10) << "Remain" << std::setw(15) << "Type" << std::setw(25) << "Timestamp" << "\n"
            << std::string(73, '-') << "\n";
        for (const auto& [price, level] : book) {
            for (const auto& o : level) {
                std::cout << std::left << std::setw(5) << o.id << std::setw(10) << o.price << std::setw(8) << o.quantity
                    << std::setw(10) << o.remaining << std::setw(15) << Utils::orderTypeToStr(o.type)
                    << std::setw(25) << Utils::formatTime(o.timestamp) << "\n";
            }
        }
    };
    print_table("Buy Orders", buyOrders);
    std::cout << "\n";
    print_table("Sell Orders", sellOrders);
}

void OrderBook::printTradeHistory() const {
    std::cout << std::left
        << std::setw(8) << "TradeID" << std::setw(8) << "BuyID" << std::setw(8) << "SellID"
        << std::setw(10) << "Price" << std::setw(8) << "Qty" << std::setw(25) << "Timestamp" << "\n"
        << std::string(67, '-') << "\n";
    for (const auto& t : trades) {
        std::cout << std::left << std::setw(8) << t.tradeId << std::setw(8) << t.buyOrderId << std::setw(8) << t.sellOrderId
            << std::setw(10) << t.price << std::setw(8) << t.quantity
            << std::setw(25) << Utils::formatTime(t.timestamp) << "\n";
    }
}

void OrderBook::expireOrders() { /* TODO */ }
void OrderBook::triggerStopOrders(double lastTradePrice) { /* TODO */ }
void OrderBook::replenishIcebergOrder(Order& order) { /* TODO */ }