#include <gtest/gtest.h>
#include "OrderBook.h"
#include <fstream>

TEST(PersistenceTest, SaveAndLoadOrderBook) {
    OrderBook book;
    book.addOrder(Order(1, OrderType::LIMIT, OrderSide::BUY, 100, 10.0));
    book.saveToFile("test_orderbook.txt");

    OrderBook loadedBook;
    loadedBook.loadFromFile("test_orderbook.txt");
    EXPECT_TRUE(loadedBook.hasOrder(1));
    std::remove("test_orderbook.txt"); // Clean up
}

TEST(PersistenceTest, HandleCorruptedFile) {
    OrderBook book;
    std::ofstream ofs("corrupted.txt");
    ofs << "garbage data";
    ofs.close();
    EXPECT_THROW(book.loadFromFile("corrupted.txt"), std::runtime_error);
    std::remove("corrupted.txt"); // Clean up
}
