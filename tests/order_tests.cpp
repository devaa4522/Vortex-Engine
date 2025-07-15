#include <gtest/gtest.h>
#include "Order.h"

TEST(OrderTest, CreateValidLimitOrder) {
    Order o(1, OrderType::LIMIT, OrderSide::BUY, 100, 123.45);
    EXPECT_EQ(o.id, 1);
    EXPECT_EQ(o.type, OrderType::LIMIT);
    EXPECT_EQ(o.side, OrderSide::BUY);
    EXPECT_EQ(o.quantity, 100);
    EXPECT_DOUBLE_EQ(o.price, 123.45);
}

TEST(OrderTest, CreateInvalidOrderThrows) {
    EXPECT_THROW(Order(2, OrderType::LIMIT, OrderSide::SELL, 0, 50.0), std::invalid_argument);
    EXPECT_THROW(Order(3, OrderType::LIMIT, OrderSide::SELL, 10, -1.0), std::invalid_argument);
}

TEST(OrderTest, SerializationAndDeserialization) {
    Order o1(4, OrderType::LIMIT, OrderSide::SELL, 50, 10.5);
    std::string ser = o1.serialize();
    Order o2 = Order::deserialize(ser);
    EXPECT_EQ(o1, o2); // You should have operator== defined for Order
}
