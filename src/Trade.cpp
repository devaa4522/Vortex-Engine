// #include "vortex/Trade.h"
// #include "vortex/Utils.h"

// using json = nlohmann::json;

// void to_json(json& j, const Trade& t) {
//     j = json{
//         {"tradeId", t.tradeId},
//         {"buyOrderId", t.buyOrderId},
//         {"sellOrderId", t.sellOrderId},
//         {"price", t.price},
//         {"quantity", t.quantity},
//         {"timestamp", t.timestamp} // Now works directly
//     };
// }

// void from_json(const json& j, Trade& t) {
//     j.at("tradeId").get_to(t.tradeId);
//     j.at("buyOrderId").get_to(t.buyOrderId);
//     j.at("sellOrderId").get_to(t.sellOrderId);
//     j.at("price").get_to(t.price);
//     j.at("quantity").get_to(t.quantity);
//     j.at("timestamp").get_to(t.timestamp); // Now works directly
// }