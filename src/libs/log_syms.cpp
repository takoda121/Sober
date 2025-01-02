#include "function_traits.hpp"
#include <cstdarg>
#include <cstdio>


enum class LogPriority : int {
    ANDROID_LOG_UNKNOWN = 0,
    ANDROID_LOG_DEFAULT,
    ANDROID_LOG_VERBOSE,
    ANDROID_LOG_DEBUG,
    ANDROID_LOG_INFO,
    ANDROID_LOG_WARN,
    ANDROID_LOG_ERROR,
    ANDROID_LOG_FATAL,
    ANDROID_LOG_SILENT
};


int __android_log_write(int, const char *tag, const char *text) {
    return printf("[log @ %s]: %s\n", tag, text);
}

int __android_log_buf_write(int, int priority, const char *tag, const char *text) {
    return __android_log_write(priority, tag, text);
}

void __android_log_assert(const char *, const char *tag, const char *fmt, ...) {
    char buffer[768];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 768, fmt, args);
    va_end(args);

    __android_log_write((int)LogPriority::ANDROID_LOG_FATAL, tag, buffer);
}

int __android_log_print(int priority, const char *tag, const char *fmt, ...) {
    char buffer[768];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, 768, fmt, args);
    va_end(args);

    return __android_log_write(priority, tag, buffer);
}


extern const std::vector<dynamic_symbol> log_symbols = {
    { "__android_log_buf_write", __android_log_buf_write },
    { "__android_log_print", __android_log_print },
    { "__android_log_write", __android_log_write },
    { "__android_log_assert", __android_log_assert }
};
