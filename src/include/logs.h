#ifndef LOGS_H
#define LOGS_H

#include <cstddef>
#include <format>
#include <iostream>
#include <span>
#include <string_view>

static constexpr std::string_view MOD_TAG = "IonFlow";

template <typename... Args>
auto format_log(std::string_view format, Args&&... args)
    -> std::string // NOLINT(cppcoreguidelines-missing-std-forward)
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

// Logs a buffer using Hex representation
void log_buffer(const std::string& name, std::span<const std::byte> buffer);

#endif // LOGS_H
