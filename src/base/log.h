#ifndef BASE_LOG_H
#define BASE_LOG_H

#include <cstdarg>
#include <cstdint>

enum LEVEL : char
{
	LEVEL_ERROR,
	LEVEL_WARN,
	LEVEL_INFO,
	LEVEL_DEBUG,
	LEVEL_TRACE,
};

struct LOG_COLOR
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

#define log_error(sys, ...) log_log(LEVEL_ERROR, sys, __VA_ARGS__)
#define log_warn(sys, ...) log_log(LEVEL_WARN, sys, __VA_ARGS__)
#define log_info(sys, ...) log_log(LEVEL_INFO, sys, __VA_ARGS__)
#define log_debug(sys, ...) log_log(LEVEL_DEBUG, sys, __VA_ARGS__)
#define log_trace(sys, ...) log_log(LEVEL_TRACE, sys, __VA_ARGS__)

#define log_error_color(color, sys, ...) log_log_color(LEVEL_ERROR, color, sys, __VA_ARGS__)
#define log_warn_color(color, sys, ...) log_log_color(LEVEL_WARN, color, sys, __VA_ARGS__)
#define log_info_color(color, sys, ...) log_log_color(LEVEL_INFO, color, sys, __VA_ARGS__)
#define log_debug_color(color, sys, ...) log_log_color(LEVEL_DEBUG, color, sys, __VA_ARGS__)
#define log_trace_color(color, sys, ...) log_log_color(LEVEL_TRACE, color, sys, __VA_ARGS__)

 

 
[[gnu::format(printf, 3, 4)]] void log_log(LEVEL level, const char *sys, const char *fmt, ...);

 
[[gnu::format(printf, 4, 5)]] void log_log_color(LEVEL level, LOG_COLOR color, const char *sys, const char *fmt, ...);

 
[[gnu::format(printf, 3, 0)]] void log_log_v(LEVEL level, const char *sys, const char *fmt, va_list args);

 
[[gnu::format(printf, 4, 0)]] void log_log_color_v(LEVEL level, LOG_COLOR color, const char *sys, const char *fmt, va_list args);

#endif  
