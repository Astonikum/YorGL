#pragma once

#include <cstdio>
#include <cstdarg>

#define FROST_LOG(level, fmt, ...) \
    frost::log(level, "frost_render", fmt, ##__VA_ARGS__)

namespace frost {

enum class LogLevel { TRACE, DEBUG, INFO, WARN, ERR };

inline const char* levelStr(LogLevel l) {
    switch (l) {
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ERR:   return "ERROR";
    }
    return "?????";
}

inline void log(LogLevel level, const char* tag, const char* fmt, ...) {
    fprintf(stderr, "[%s] [%s] ", levelStr(level), tag);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
}

} // namespace frost

#define LOG_TRACE(fmt, ...) FROST_LOG(frost::LogLevel::TRACE, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) FROST_LOG(frost::LogLevel::DEBUG, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  FROST_LOG(frost::LogLevel::INFO,  fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  FROST_LOG(frost::LogLevel::WARN,  fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) FROST_LOG(frost::LogLevel::ERR,   fmt, ##__VA_ARGS__)
