#ifndef LOGS_H
#define LOGS_H

#include <span>
#include <string>

enum class LogLevel { DEBUG, INFO, WARNING, ERROR };

void log_debug(const char* format, ...);
void log_info(const char* format, ...);
void log_warning(const char* format, ...);
void log_error(const char* format, ...);
void log_buffer(const std::string& named, const std::span<const std::byte> buffer);

#define LOG_TRACE() log_debug("-> %s():\n", __FUNCTION__)

#endif // !LOGS_H
