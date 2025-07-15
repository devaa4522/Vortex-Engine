#include "vortex/matching_engine.h"
#include "vortex/Utils.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <string>
#include <iomanip>
#include <chrono>

// Helper: print available commands
void printHelp() {
    std::cout << "Commands:\n";
    std::cout << "  add <side> <type> <price> <quantity> [peakSize] [stopPrice] [expiry(YYYY-MM-DDTHH:MM)]\n";
    std::cout << "      side: buy|sell\n";
    std::cout << "      type: limit|market|stop|iceberg|fok|ioc\n";
    std::cout << "      price: order price (set to 0 for market orders)\n";
    std::cout << "      quantity: order quantity\n";
    std::cout << "      peakSize: visible size for iceberg (required for iceberg, else 0)\n";
    std::cout << "      stopPrice: stop trigger price (required for stop, else 0)\n";
    std::cout << "      expiry: optional expiry time (YYYY-MM-DDTHH:MM)\n";
    std::cout << "  cancel <orderId>\n";
    std::cout << "  modify <orderId> <new_price> <new_quantity>\n";
    std::cout << "  book\n";
    std::cout << "  trades\n";
    std::cout << "  save <filename>\n";
    std::cout << "  load <filename>\n";
    std::cout << "  autosave on|off\n";
    std::cout << "  help\n";
    std::cout << "  quit\n";
}

OrderSide parseSide(const std::string& s) {
    if (s == "buy" || s == "b") return OrderSide::Buy;
    if (s == "sell"|| s == "s") return OrderSide::Sell;
    throw std::invalid_argument("Invalid order side");
}

OrderType parseType(const std::string& s) {
    if (s == "limit"    || s == "l")   return OrderType::Limit;
    if (s == "market"   || s == "m")   return OrderType::Market;
    if (s == "stop")                   return OrderType::Stop;
    if (s == "iceberg")                return OrderType::Iceberg;
    if (s == "fok")                    return OrderType::FillOrKill;
    if (s == "ioc")                    return OrderType::ImmediateOrCancel;
    throw std::invalid_argument("Invalid order type");
}

std::string toLower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), ::tolower);
    return out;
}

std::chrono::system_clock::time_point parseExpiry(const std::string& s) {
    if (s.empty()) return std::chrono::system_clock::time_point();
    std::tm tm = {};
    std::istringstream ss(s);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M");
    if (ss.fail()) return std::chrono::system_clock::time_point();
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

int main() {
    MatchingEngine engine;
    std::string line;
    bool autosaveEnabled = false;
    std::string autosaveFile = "autosave.txt";

    std::cout << "Vortex Engine CLI\n";
    printHelp();
    std::cout << "Order IDs are shown after adding an order and are required for cancellation or modification.\n";

    while (true) {
        std::cout << "\n> ";
        if (!std::getline(std::cin, line)) break;
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;
        cmd = toLower(cmd);
        try {
            if (cmd == "add") {
                std::string sideStr, typeStr, expiryStr;
                double price = 0, stopPrice = 0;
                uint64_t qty = 0, peakSize = 0;
                iss >> sideStr >> typeStr >> price >> qty;

                sideStr = toLower(sideStr);
                typeStr = toLower(typeStr);

                if (sideStr.empty() || typeStr.empty() || qty == 0 ||
                    (typeStr == "limit" && price <= 0) ||
                    (typeStr == "iceberg" && !((iss >> peakSize) && peakSize > 0)) ||
                    (typeStr == "stop" && !((iss >> stopPrice) && stopPrice > 0))) {
                    std::cerr << "Usage: add <side> <type> <price> <quantity> [peakSize] [stopPrice] [expiry]\n";
                    continue;
                }

                // Parse optional peakSize and stopPrice for advanced types
                if (typeStr == "iceberg" && peakSize == 0) {
                    std::cerr << "Iceberg orders require peakSize > 0\n";
                    continue;
                }
                if (typeStr == "stop" && stopPrice == 0) {
                    std::cerr << "Stop orders require stopPrice > 0\n";
                    continue;
                }
                if ((typeStr != "iceberg") && (iss >> peakSize)) {} // ignore peakSize for non-iceberg
                if ((typeStr != "stop") && (iss >> stopPrice)) {}   // ignore stopPrice for non-stop

                // Optional expiry
                iss >> expiryStr;
                std::chrono::system_clock::time_point expiry = parseExpiry(expiryStr);

                OrderSide side = parseSide(sideStr);
                OrderType type = parseType(typeStr);

                // For non-iceberg, set peakSize=0; for non-stop, set stopPrice=0
                if (type != OrderType::Iceberg) peakSize = 0;
                if (type != OrderType::Stop) stopPrice = 0;

                // Convert expiry to seconds from now (uint64_t)
                uint64_t expirySec = 0;
                if (expiry != std::chrono::system_clock::time_point()) {
                    auto now = std::chrono::system_clock::now();
                    expirySec = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(expiry - now).count());
                    if (expirySec < 0) expirySec = 0;
                }

                uint64_t orderId = engine.addOrder(side, type, price, stopPrice, qty, peakSize, expirySec);
                if (orderId != 0) {
                    std::cout << "Order added to book with ID: " << orderId << std::endl;
                    if (autosaveEnabled) engine.save(autosaveFile);
                } else {
                    std::cout << "Order fully matched (not resting in book) or invalid parameters." << std::endl;
                }
            } else if (cmd == "trades") {
                engine.printTradeHistory();
            } else if (cmd == "book") {
                engine.printOrderBook();
            } else if (cmd == "cancel") {
                uint64_t orderId = 0;
                iss >> orderId;
                if (orderId == 0) {
                    std::cerr << "Usage: cancel <orderId>\n";
                    continue;
                }
                if (engine.cancelOrder(orderId)) {
                    std::cout << "Order " << orderId << " cancelled.\n";
                    if (autosaveEnabled) engine.save(autosaveFile);
                } else {
                    std::cout << "Order " << orderId << " not found or already filled.\n";
                }
            } else if (cmd == "modify") {
                uint64_t orderId = 0;
                double newPrice = 0;
                uint64_t newQty = 0;
                iss >> orderId >> newPrice >> newQty;
                if (orderId == 0 || newPrice <= 0 || newQty == 0) {
                    std::cerr << "Usage: modify <orderId> <new_price> <new_quantity>\n";
                    continue;
                }
                if (engine.modifyOrder(orderId, newPrice, newQty)) {
                    std::cout << "Order " << orderId << " modified.\n";
                    if (autosaveEnabled) engine.save(autosaveFile);
                } else {
                    std::cout << "Order " << orderId << " not found or already filled.\n";
                }
            } else if (cmd == "save") {
                std::string filename;
                iss >> filename;
                if (filename.empty()) {
                    std::cerr << "Usage: save <filename>\n";
                    continue;
                }
                engine.save(filename);
                std::cout << "Order book and trades saved to " << filename << "\n";
            } else if (cmd == "load") {
                std::string filename;
                iss >> filename;
                if (filename.empty()) {
                    std::cerr << "Usage: load <filename>\n";
                    continue;
                }
                engine.load(filename);
                std::cout << "Order book and trades loaded from " << filename << "\n";
            } else if (cmd == "autosave") {
                std::string arg;
                iss >> arg;
                if (arg == "on") {
                    autosaveEnabled = true;
                    std::cout << "Autosave enabled (file: " << autosaveFile << ")\n";
                } else if (arg == "off") {
                    autosaveEnabled = false;
                    std::cout << "Autosave disabled\n";
                } else {
                    std::cout << "Usage: autosave on|off\n";
                }
            } else if(cmd == "help") {
                printHelp();
            } else if (cmd == "quit") {
                break;
            } else if (!cmd.empty()) {
                std::cerr << "Unknown command: " << cmd << "\n";
            }
        } catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << "\n";
        }
    }
    return 0;
}
