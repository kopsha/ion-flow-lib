#include "console.h"

#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <span>
#include <sstream>
#include <string>
#include <string_view>

#ifdef __ANDROID__
  #include <android/log.h>
#endif

namespace
{
constexpr std::string MOD_TAG = "ions";
}

namespace console
{

void log_message(Level level, std::string_view message)
{
    const auto ndx = static_cast<size_t>(level);

#ifdef __ANDROID__
    static const std::array<android_LogPriority, 4> asLogId {
        ANDROID_LOG_DEBUG,
        ANDROID_LOG_INFO,
        ANDROID_LOG_WARN,
        ANDROID_LOG_ERROR,
    };
    __android_log_print(asLogId[ndx], MOD_TAG.data(), "%s", message.data());
#else
    static const std::array<std::string_view, 4> asStr {
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR",
    };
    std::string line = std::format("[{}] {}", asStr.at(ndx), message);
    std::cout << line << '\n';
#endif
}

void buffer(std::string_view name, std::span<const uint8_t> buffer)
{
    std::ostringstream oss;
    oss << name << " (" << buffer.size() << " bytes): ";

    for (const auto& value : buffer)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(value)
            << " ";
    }

    log_message(Level::DEBUG, oss.str());
}
} // namespace console
