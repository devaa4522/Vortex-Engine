#include <gtest/gtest.h>
#include "matching_engine.h"

TEST(MatchingEngineTest, FullOrderLifecycle) {
    MatchingEngine engine;
    int id = engine.addOrder(OrderType::LIMIT, OrderSide::BUY, 100, 10.0);
    EXPECT_TRUE(engine.modifyOrder(id, 120, 9.5));
    EXPECT_TRUE(engine.cancelOrder(id));
    EXPECT_FALSE(engine.cancelOrder(id)); // Already cancelled
}

TEST(MatchingEngineTest, MarketOrderMatch) {
    MatchingEngine engine;
    engine.addOrder(OrderType::LIMIT, OrderSide::SELL, 100, 10.0);
    int marketBuyId = engine.addOrder(OrderType::MARKET, OrderSide::BUY, 100, 0.0);
    auto trades = engine.getRecentTrades();
    ASSERT_FALSE(trades.empty());
    EXPECT_EQ(trades.back().quantity, 100);
    EXPECT_DOUBLE_EQ(trades.back().price, 10.0);
}
