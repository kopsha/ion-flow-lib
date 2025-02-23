#pragma once

#include <cstdint>
#include <format>
#include <span>
#include <string_view>

namespace console
{

enum class Level : uint8_t
{
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR
};

void log_message(Level level, std::string_view message);
void buffer(std::string_view name, std::span<const uint8_t> buffer);

#ifdef NDEBUG
  #define CONSOLE_TRACE() ((void)0)
#else
  #define CONSOLE_TRACE(...) console::debug("{}({})", __PRETTY_FUNCTION__, __VA_ARGS__)
#endif

// NOLINTBEGIN(cppcoreguidelines-missing-std-forward)
template <typename... Args> void debug(std::string_view format, Args&&... args)
{
    log_message(Level::DEBUG, std::vformat(format, (std::make_format_args(args...))));
}

template <typename... Args> void info(std::string_view format, Args&&... args)
{
    log_message(Level::INFO, std::vformat(format, (std::make_format_args(args...))));
}

template <typename... Args> void warning(std::string_view format, Args&&... args)
{
    log_message(Level::WARNING, std::vformat(format, (std::make_format_args(args...))));
}

template <typename... Args> void error(std::string_view format, Args&&... args)
{
    log_message(Level::ERROR, std::vformat(format, (std::make_format_args(args...))));
}

// NOLINTEND(cppcoreguidelines-missing-std-forward)

} // namespace console
