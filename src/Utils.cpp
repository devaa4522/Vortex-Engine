#include "vortex/Utils.h"
#include "vortex/Order.h" // Include full Order definition here
#include <sstream>
#include <iomanip>
#include <ctime>

namespace Utils {

std::string formatTime(const std::chrono::system_clock::time_point& tp) {
    if (tp == std::chrono::system_clock::time_point::min()) return "N/A";
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()) % 1000;
    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::chrono::system_clock::time_point parseTime(const std::string& s) {
    if (s == "N/A") return std::chrono::system_clock::time_point::min();
    std::tm tm = {};
    std::istringstream ss(s);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    auto time = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    if (ss.peek() == '.') {
        ss.ignore();
        long long ms;
        ss >> ms;
        time += std::chrono::milliseconds(ms);
    }
    return time;
}

std::chrono::system_clock::time_point now() {
    return std::chrono::system_clock::now();
}

std::string orderTypeToStr(OrderType type) {
    switch (type) {
        case OrderType::Limit: return "Limit";
        case OrderType::Market: return "Market";
        case OrderType::Stop: return "Stop";
        case OrderType::Iceberg: return "Iceberg";
        case OrderType::FillOrKill: return "FillOrKill";
        case OrderType::ImmediateOrCancel: return "ImmediateOrCancel";
        default: return "Unknown";
    }
}

std::string orderStatusToStr(OrderStatus status) {
    switch (status) {
        case OrderStatus::Active: return "Active";
        case OrderStatus::Filled: return "Filled";
        case OrderStatus::Cancelled: return "Cancelled";
        case OrderStatus::Expired: return "Expired";
        case OrderStatus::Pending: return "Pending";
        default: return "Unknown";
    }
}

}