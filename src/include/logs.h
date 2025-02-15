#ifndef LOGS_H
#define LOGS_H

#include <cstddef>
#include <span>
#include <string_view>

template <typename... Args> void log_debug(std::string_view format, Args&&... args);

template <typename... Args> void log_info(std::string_view format, Args&&... args);

template <typename... Args> void log_warning(std::string_view format, Args&&... args);

template <typename... Args> void log_error(std::string_view format, Args&&... args);

// Logs a buffer using Hex representation
void log_buffer(const std::string& name, std::span<const std::byte> buffer);

#endif // LOGS_H
