#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <time.h>
#include <stdarg.h>

// Log levels
typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} log_level_t;

// Log output options
typedef struct {
    bool enable_timestamp;
    bool enable_level;
    bool enable_file_line;
    bool enable_color;
    log_level_t min_level;
    FILE* output_stream;
} logger_config_t;

// Initialize logger
void logger_init(logger_config_t* config);

// Set minimum log level
void logger_set_level(log_level_t level);

// Log functions
void logger_log(log_level_t level, const char* file, int line, const char* format, ...);

// Convenience macros
#define LOG_DEBUG(format, ...) logger_log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...)  logger_log(LOG_LEVEL_INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...)  logger_log(LOG_LEVEL_WARN, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) logger_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...) logger_log(LOG_LEVEL_FATAL, __FILE__, __LINE__, format, ##__VA_ARGS__)

// Helper functions
const char* logger_level_to_string(log_level_t level);
const char* logger_level_to_color(log_level_t level);
void logger_hex_dump(const char* label, const void* data, size_t size);

#endif