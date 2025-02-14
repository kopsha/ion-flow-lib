#ifndef LOGS_H
#define LOGS_H

#include <cstdint>
#include <span>
#include <string>

enum class LogLevel : uint8_t { DEBUG, INFO, WARNING, ERROR };

void log_debug(const char* format, ...);
void log_info(const char* format, ...);
void log_warning(const char* format, ...);
void log_error(const char* format, ...);
void log_buffer(const std::string& named, std::span<const std::byte> buffer);

#define LOG_TRACE() log_debug("-> %s():\n", __FUNCTION__)

#endif // !LOGS_H
