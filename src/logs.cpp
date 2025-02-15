#include "logs.h"

#include <cstddef>
#include <format>
#include <iomanip>
#include <iostream>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#ifdef __ANDROID__
  #include <android/log.h>
#endif

static constexpr std::string_view MOD_TAG = "IonFlow";

namespace {

template <typename... Args>
auto format_log(std::string_view format, Args&&... args) -> std::string // NOLINT(cppcoreguidelines-missing-std-forward)
{
    auto format_args = std::make_format_args(args...);
    return std::vformat(std::string(format), format_args);
}

template <typename... Args>
void log_message(std::string_view level, std::string_view format, Args&&... args)
{
    const std::string message = format_log(format, std::forward<Args>(args)...);

#ifdef __ANDROID__
    int priority = ANDROID_LOG_DEFAULT;
    if (level == "DEBUG")
        priority = ANDROID_LOG_DEBUG;
    else if (level == "INFO")
        priority = ANDROID_LOG_INFO;
    else if (level == "WARN")
        priority = ANDROID_LOG_WARN;
    else if (level == "ERROR")
        priority = ANDROID_LOG_ERROR;

    __android_log_print(priority, MOD_TAG.data(), "%s", message.c_str());
#else
    std::ostream& output = (level == "ERROR" || level == "WARN") ? std::cerr : std::cout;
    output << "[" << MOD_TAG << "] " << level << ": " << message << '\n';
#endif
}

} // anonymous namespace

// Specific logging functions
template <typename... Args> void log_debug(std::string_view format, Args&&... args)
{
#ifndef NDEBUG
    log_message("DEBUG", format, std::forward<Args>(args)...);
#endif
}

template <typename... Args> void log_info(std::string_view format, Args&&... args)
{
    log_message("INFO", format, std::forward<Args>(args)...);
}

template <typename... Args> void log_warning(std::string_view format, Args&&... args)
{
    log_message("WARN", format, std::forward<Args>(args)...);
}

template <typename... Args> void log_error(std::string_view format, Args&&... args)
{
    log_message("ERROR", format, std::forward<Args>(args)...);
}

// Function to log binary buffer content in hex format
void log_buffer(const std::string& name, std::span<const std::byte> buffer)
{
    std::ostringstream oss;
    oss << name << " (" << buffer.size() << " bytes): ";

    for (const std::byte& byte : buffer) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte)
            << " ";
    }

    log_debug("{}", oss.str());
}
