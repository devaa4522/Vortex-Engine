#include "matching_engine.h"
#include <iostream>
#include <sstream>
#include <string>

int main() {
    MatchingEngine engine;
    int orderId = 1;
    std::cout << "Simple Matching Engine\n";
    std::cout << "Enter orders in format: side type price quantity\n";
    std::cout << "Example: buy limit 100.5 10\n";
    std::cout << "Type 'exit' to quit.\n\n";
    while (true) {
        std::cout << "Order> ";
        std::string input;
        std::getline(std::cin, input);
        if (input == "exit") break;

        std::istringstream iss(input);
        std::string side, type;
        double price = 0.0;
        int quantity = 0;
        iss >> side >> type >> price >> quantity;

        if (side.empty() || type.empty() || quantity <= 0 ||
            (type == "limit" && price <= 0.0)) {
            std::cout << "Invalid input. Try again.\n";
            continue;
        }

        Order order{orderId++, side, type, price, quantity, std::chrono::high_resolution_clock::now()};
        engine.processOrder(order);
        engine.printOrderBook();
    }
    return 0;
}
