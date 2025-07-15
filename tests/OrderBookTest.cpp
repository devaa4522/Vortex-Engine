#include "gtest/gtest.h"
#include "../src/OrderBook.h"
#include "../src/Utils.h"

// Test basic limit order matching
TEST(OrderBookTest, LimitOrderMatching) {
    OrderBook ob;
    Order buy;
    buy.id = 0;
    buy.side = OrderSide::Buy;
    buy.type = OrderType::Limit;
    buy.price = 100.0;
    buy.quantity = 10;
    buy.remaining = 10;
    buy.timestamp = Utils::now();
    buy.status = OrderStatus::Active;

    Order sell = buy;
    sell.side = OrderSide::Sell;
    sell.price = 100.0;

    ob.addOrder(buy);
    ob.addOrder(sell);

    ASSERT_EQ(ob.trades.size(), 1);
    ASSERT_EQ(ob.trades[0].price, 100.0);
    ASSERT_EQ(ob.trades[0].quantity, 10);
}

// Test stop order triggering
TEST(OrderBookTest, StopOrderTriggered) {
    OrderBook ob;
    Order stopBuy;
    stopBuy.id = 0;
    stopBuy.side = OrderSide::Buy;
    stopBuy.type = OrderType::Stop;
    stopBuy.price = 101.0;
    stopBuy.stopPrice = 101.0;
    stopBuy.quantity = 5;
    stopBuy.remaining = 5;
    stopBuy.timestamp = Utils::now();
    stopBuy.status = OrderStatus::Pending;

    ob.addOrder(stopBuy);

    // Simulate market price reaching stop price
    ob.triggerStopOrders(101.0);

    // The order should now be active in the book
    bool found = false;
    for (const auto& [id, order] : ob.allOrders) {
        if ((order.type == OrderType::Market || order.type == OrderType::Limit) && order.status == OrderStatus::Active)
            found = true;
    }
    ASSERT_TRUE(found);
}
    



// #include "gtest/gtest.h"
// #include "../src/OrderBook.h"
// #include "../src/Utils.h"

// TEST(OrderBookTest, LimitOrderMatching) {
//     OrderBook ob;
//     Order buy;
//     buy.id = 0;
//     buy.side = OrderSide::Buy;
//     buy.type = OrderType::Limit;
//     buy.price = 100.0;
//     buy.quantity = 10;
//     buy.remaining = 10;
//     buy.timestamp = Utils::now();
//     buy.status = OrderStatus::Active;

//     Order sell = buy;
//     sell.side = OrderSide::Sell;
//     sell.price = 100.0;

//     ob.addOrder(buy);
//     ob.addOrder(sell);

//     ASSERT_EQ(ob.trades.size(), 1);
//     ASSERT_EQ(ob.trades[0].price, 100.0);
//     ASSERT_EQ(ob.trades[0].quantity, 10);
// }

// TEST(OrderBookTest, StopOrderTriggered) {
//     OrderBook ob;
//     Order stopBuy;
//     stopBuy.id = 0;
//     stopBuy.side = OrderSide::Buy;
//     stopBuy.type = OrderType::Stop;
//     stopBuy.price = 101.0;
//     stopBuy.stopPrice = 101.0;
//     stopBuy.quantity = 5;
//     stopBuy.remaining = 5;
//     stopBuy.timestamp = Utils::now();
//     stopBuy.status = OrderStatus::Pending;

//     ob.addOrder(stopBuy);

//     // Simulate market price reaching stop price
//     ob.triggerStopOrders(101.0);

//     // The order should now be active in the book
//     bool found = false;
//     for (const auto& [id, order] : ob.allOrders) {
//         if ((order.type == OrderType::Market || order.type == OrderType::Limit) && order.status == OrderStatus::Active)
//             found = true;
//     }
//     ASSERT_TRUE(found);
// }

// TEST(OrderBookTest, IcebergOrderReplenishment) {
//     OrderBook ob;
//     Order iceberg;
//     iceberg.id = 0;
//     iceberg.side = OrderSide::Buy;
//     iceberg.type = OrderType::Iceberg;
//     iceberg.price = 100.0;
//     iceberg.quantity = 20;
//     iceberg.remaining = 20;
//     iceberg.peakSize = 5;
//     iceberg.visibleQuantity = 5;
//     iceberg.timestamp = Utils::now();
//     iceberg.status = OrderStatus::Active;

//     ob.addOrder(iceberg);

//     // Simulate a partial fill
//     ob.buyOrders.front().visibleQuantity -= 5;
//     ob.buyOrders.front().remaining -= 5;
//     ob.replenishIcebergOrder(ob.buyOrders.front());

//     ASSERT_EQ(ob.buyOrders.front().visibleQuantity, 5);
//     ASSERT_EQ(ob.buyOrders.front().remaining, 15);
// }

// TEST(OrderBookTest, FOKOrderNotFillable) {
//     OrderBook ob;
//     Order fok;
//     fok.id = 0;
//     fok.side = OrderSide::Buy;
//     fok.type = OrderType::FillOrKill;
//     fok.price = 100.0;
//     fok.quantity = 10;
//     fok.remaining = 10;
//     fok.timestamp = Utils::now();
//     fok.status = OrderStatus::Active;

//     // No matching sell order, should cancel
//     ob.addOrder(fok);

//     ASSERT_EQ(ob.allOrders.begin()->second.status, OrderStatus::Cancelled);
// }

// TEST(OrderBookTest, IOCOrderPartialFill) {
//     OrderBook ob;
//     // Add a sell order
//     Order sell;
//     sell.id = 0;
//     sell.side = OrderSide::Sell;
//     sell.type = OrderType::Limit;
//     sell.price = 100.0;
//     sell.quantity = 5;
//     sell.remaining = 5;
//     sell.timestamp = Utils::now();
//     sell.status = OrderStatus::Active;
//     ob.addOrder(sell);

//     // Add IOC buy order for 10
//     Order ioc;
//     ioc.id = 0;
//     ioc.side = OrderSide::Buy;
//     ioc.type = OrderType::ImmediateOrCancel;
//     ioc.price = 100.0;
//     ioc.quantity = 10;
//     ioc.remaining = 10;
//     ioc.timestamp = Utils::now();
//     ioc.status = OrderStatus::Active;

//     ob.addOrder(ioc);

//     // Only 5 should be filled, remainder cancelled
//     ASSERT_EQ(ob.trades.size(), 1);
//     ASSERT_EQ(ob.trades[0].quantity, 5);
//     // Find IOC order in allOrders and check status
//     for (const auto& [id, order] : ob.allOrders) {
//         if (order.type == OrderType::ImmediateOrCancel)
//             ASSERT_EQ(order.status, OrderStatus::Cancelled);
//     }
// }
