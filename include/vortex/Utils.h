#pragma once
#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

// Forward-declare enums to break circular dependency
enum class OrderType;
enum class OrderStatus;

// Teach nlohmann::json how to serialize std::chrono::system_clock::time_point
namespace nlohmann {
    template <>
    struct adl_serializer<std::chrono::system_clock::time_point> {
        static void to_json(json& j, const std::chrono::system_clock::time_point& tp) {
            if (tp == std::chrono::system_clock::time_point::min()) {
                j = nullptr;
            } else {
                j = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
            }
        }

        static void from_json(const json& j, std::chrono::system_clock::time_point& tp) {
            if (j.is_null()) {
                tp = std::chrono::system_clock::time_point::min();
            } else {
                tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(j.get<long long>()));
            }
        }
    };
}

namespace Utils {
    std::string formatTime(const std::chrono::system_clock::time_point& tp);
    std::chrono::system_clock::time_point parseTime(const std::string& s);
    std::chrono::system_clock::time_point now();
    std::string orderTypeToStr(OrderType type);
    std::string orderStatusToStr(OrderStatus status);
}