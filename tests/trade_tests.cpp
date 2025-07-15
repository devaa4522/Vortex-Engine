#include <gtest/gtest.h>
#include "Trade.h"

TEST(TradeTest, CreateAndSerializeTrade) {
    Trade t(1, 2, 3, 100, 99.99, 1650000000);
    EXPECT_EQ(t.id, 1);
    EXPECT_EQ(t.buyOrderId, 2);
    EXPECT_EQ(t.sellOrderId, 3);
    EXPECT_EQ(t.quantity, 100);
    EXPECT_DOUBLE_EQ(t.price, 99.99);
    EXPECT_EQ(t.timestamp, 1650000000);

    std::string ser = t.serialize();
    Trade t2 = Trade::deserialize(ser);
    EXPECT_EQ(t, t2); // You should have operator== defined for Trade
}
