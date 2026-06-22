#pragma once

#include <cstdio>

namespace yorgl {

enum class LogLevel { Trace, Debug, Info, Warn, Error };

inline const char* logLevelName(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warn: return "WARN";
        case LogLevel::Error: return "ERROR";
    }
    return "INFO";
}

template <typename... Args>
inline void log(LogLevel level, const char* fmt, Args... args) {
    std::fprintf(stderr, "[YorGL][%s] ", logLevelName(level));
    std::fprintf(stderr, fmt, args...);
    std::fprintf(stderr, "\n");
}

} // namespace yorgl

#define LOG_TRACE(fmt, ...) ::yorgl::log(::yorgl::LogLevel::Trace, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) ::yorgl::log(::yorgl::LogLevel::Debug, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  ::yorgl::log(::yorgl::LogLevel::Info, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  ::yorgl::log(::yorgl::LogLevel::Warn, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) ::yorgl::log(::yorgl::LogLevel::Error, fmt, ##__VA_ARGS__)
