#include "logs.h"

#include <cstdarg>
#include <cstdio>
#include <vector>
#ifdef __ANDROID__
  #include <android/log.h>
#endif

static constexpr char* MOD_TAG = "IonFlow";

void log_debug(const char* format, ...)
{
#if !defined(NDEBUG)
    va_list args;
    va_start(args, format);

  #ifdef __ANDROID__
    __android_log_vprint(ANDROID_LOG_DEBUG, MOD_TAG, format, args);
  #else
    printf("[%s] DEBUG: ", MOD_TAG);
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
    __android_log_vprint(ANDROID_LOG_INFO, MOD_TAG, format, args);
#else
    printf("[%s] INFO: ", MOD_TAG);
    vprintf(format, args);
#endif
    va_end(args);
}

void log_warning(const char* format, ...)
{
    va_list args;
    va_start(args, format);

#ifdef __ANDROID__
    __android_log_vprint(ANDROID_LOG_WARN, MOD_TAG, format, args);
#else
    fprintf(stderr, "[%s] WARN: ", MOD_TAG);
    vfprintf(stderr, format, args);
#endif
    va_end(args);
}

void log_error(const char* format, ...)
{
    va_list args;
    va_start(args, format);

#ifdef __ANDROID__
    __android_log_vprint(ANDROID_LOG_ERROR, MOD_TAG, format, args);
#else
    fprintf(stderr, "[%s] ERROR: ", MOD_TAG);
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
