#include "vortex/matching_engine.h"
#include "vortex/Utils.h"
#include <crow.h>
#include <nlohmann/json.hpp>
#include <mutex>
#include <thread>
#include <unordered_set>

using json = nlohmann::json;
using crow::request;
using crow::response;
using crow::SimpleApp;

class ApiServer {
public:
    ApiServer(MatchingEngine& me) : engine(me) {}

    void run(int port = 8080) {
        defineRestEndpoints();
        defineWebSocketEndpoint();
        spawnBroadcastThread();
        app.port(static_cast<uint16_t>(port)).multithreaded().run();
    }

private:
    SimpleApp app;
    MatchingEngine& engine; // Use a reference to the main engine

    std::mutex ws_mtx;
    std::unordered_set<crow::websocket::connection*> ws_clients;

    void defineRestEndpoints() {
        CROW_ROUTE(app, "/api/v1/orders").methods("POST"_method)
        ([this](const request& req) {
            try {
                auto j = json::parse(req.body);
                auto side      = j.at("side").get<OrderSide>();
                auto type      = j.at("type").get<OrderType>();
                double price   = j.value("price", 0.0);
                double stopP   = j.value("stopPrice", 0.0);
                auto qty       = j.at("quantity").get<uint64_t>();
                auto peak      = j.value("peakSize", 0ULL);
                auto expiry    = j.value("expirySec", 0ULL);
                
                // Post the work to the queue instead of processing it here
                engine.postOrder(side, type, price, stopP, qty, peak, expiry);
                
                // Respond immediately
                return response{202, json{{"status", "accepted"}}.dump()};
            }
            catch (const json::exception& e) {
                return response{400, json{{"error", std::string("JSON Parsing Error: ") + e.what()}}.dump()};
            }
            catch (const std::exception& ex) {
                return response{500, json{{"error", ex.what()}}.dump()};
            }
        });

        // Other GET routes remain the same as they are read-only
        CROW_ROUTE(app, "/api/v1/orders/<uint>")
        ([this](uint64_t id) {
            auto ord = engine.getOrderById(id);
            if (!ord) return response{404, R"({"error":"Order not found"})"};
            json j = *ord;
            return response{j.dump()};
        });
        CROW_ROUTE(app, "/api/v1/orderbook")
        ([this] { return response{engine.getOrderBookSnapshot().dump()}; });
        CROW_ROUTE(app, "/api/v1/trades")
        ([this] { return response{engine.getTradeHistory().dump()}; });
    }

    void defineWebSocketEndpoint() {
        CROW_ROUTE(app, "/api/v1/ws").websocket(&app)
        .onopen([this](crow::websocket::connection& c) {
            std::lock_guard lk(ws_mtx);
            ws_clients.insert(&c);
        })
        .onclose([this](crow::websocket::connection& c, const std::string&, uint16_t) {
            std::lock_guard lk(ws_mtx);
            ws_clients.erase(&c);
        })
        .onmessage([](crow::websocket::connection&, const std::string&, bool) {});
    }

    void spawnBroadcastThread() {
        std::thread([this] {
            while (true) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                auto payload = json{
                    {"type", "snapshot"},
                    {"orderBook", engine.getOrderBookSnapshot()},
                    {"trades",    engine.getTradeHistory()}
                };
                auto msg = payload.dump();
                std::lock_guard lk(ws_mtx);
                for (auto* c : ws_clients) {
                    if (c) c->send_text(msg);
                }
            }
        }).detach();
    }
};

int main(int argc, char* argv[]) {
    try {
        // Create a single matching engine
        MatchingEngine engine;
        // Start its dedicated processing thread
        engine.run();

        // Pass a reference to the engine to the API server
        ApiServer server(engine);
        
        int port = 8080;
        if (argc > 1) port = std::stoi(argv[1]);
        server.run(port);
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}