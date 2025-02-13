#include "logs.h"

#include <cstdarg>
#include <vector>
#ifdef __ANDROID__
  #include <android/log.h>
#endif

#define RCA "rcAgent"

void log_debug(const char* format, ...)
{
#if !defined(NDEBUG)
    va_list args;
    va_start(args, format);

  #ifdef __ANDROID__
    __android_log_vprint(ANDROID_LOG_DEBUG, RCA, format, args);
  #else
    printf("[%s] DEBUG: ", RCA);
    vprintf(format, args);
  #endif
    va_end(args);
#endif
}

void log_info(const char* format, ...)
{
    va_list args;
    va_start(args, format);

#ifdef __ANDROID__
    __android_log_vprint(ANDROID_LOG_INFO, RCA, format, args);
#else
    printf("[%s] INFO: ", RCA);
    vprintf(format, args);
#endif
    va_end(args);
}

void log_warning(const char* format, ...)
{
    va_list args;
    va_start(args, format);

#ifdef __ANDROID__
    __android_log_vprint(ANDROID_LOG_WARN, RCA, format, args);
#else
    fprintf(stderr, "[%s] WARN: ", RCA);
    vfprintf(stderr, format, args);
#endif
    va_end(args);
}

void log_error(const char* format, ...)
{
    va_list args;
    va_start(args, format);

#ifdef __ANDROID__
    __android_log_vprint(ANDROID_LOG_ERROR, RCA, format, args);
#else
    fprintf(stderr, "[%s] ERROR: ", RCA);
    vfprintf(stderr, format, args);
#endif
    va_end(args);
}

void log_buffer(const std::string& named, const std::span<const std::byte> buffer)
{
    size_t buffer_size = named.size() + 20 + buffer.size() * 3;
    std::vector<char> output(buffer_size);
    int offset = sprintf(output.data(), "%s (%zu bytes): ", named.c_str(), buffer.size());
    for (size_t i = 0; i < buffer.size(); i++) {
        offset += sprintf(
            output.data() + offset, "%02x ", static_cast<unsigned char>(buffer[i])
        );
    }

    log_debug("%s\n", output.data());
}
