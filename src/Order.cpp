// #include "vortex/Order.h"
// #include "vortex/Utils.h"

// using json = nlohmann::json;

// void to_json(json& j, const Order& o) {
//     j = json{
//         {"id", o.id},
//         {"side", o.side},
//         {"type", o.type},
//         {"price", o.price},
//         {"stopPrice", o.stopPrice},
//         {"quantity", o.quantity},
//         {"remaining", o.remaining},
//         {"peakSize", o.peakSize},
//         {"visibleQuantity", o.visibleQuantity},
//         {"timestamp", o.timestamp}, // Now works directly
//         {"expiry", o.expiry},       // Now works directly
//         {"status", o.status},
//         {"auditTrail", o.auditTrail}
//     };
// }

// void from_json(const json& j, Order& o) {
//     j.at("id").get_to(o.id);
//     j.at("side").get_to(o.side);
//     j.at("type").get_to(o.type);
//     j.at("price").get_to(o.price);
//     j.at("stopPrice").get_to(o.stopPrice);
//     j.at("quantity").get_to(o.quantity);
//     j.at("remaining").get_to(o.remaining);
//     j.at("peakSize").get_to(o.peakSize);
//     j.at("visibleQuantity").get_to(o.visibleQuantity);
//     j.at("timestamp").get_to(o.timestamp); // Now works directly
//     j.at("expiry").get_to(o.expiry);       // Now works directly
//     j.at("status").get_to(o.status);
//     j.at("auditTrail").get_to(o.auditTrail);
// }