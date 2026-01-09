#include "logger.h"
#include <string.h>

// Default configuration
static logger_config_t logger_config = {
    .enable_timestamp = true,
    .enable_level = true,
    .enable_file_line = false,
    .enable_color = true,
    .min_level = LOG_LEVEL_INFO,
    .output_stream = NULL
};

void logger_init(logger_config_t* config) {
    if (config) {
        memcpy(&logger_config, config, sizeof(logger_config_t));
    }
    
    if (logger_config.output_stream == NULL) {
        logger_config.output_stream = stdout;
    }
    
    printf("[LOGGER] Initialized (min level: %s)\n", 
           logger_level_to_string(logger_config.min_level));
}

void logger_set_level(log_level_t level) {
    logger_config.min_level = level;
    printf("[LOGGER] Log level set to: %s\n", logger_level_to_string(level));
}

const char* logger_level_to_string(log_level_t level) {
    switch (level) {
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_INFO:  return "INFO";
        case LOG_LEVEL_WARN:  return "WARN";
        case LOG_LEVEL_ERROR: return "ERROR";
        case LOG_LEVEL_FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

const char* logger_level_to_color(log_level_t level) {
    if (!logger_config.enable_color) {
        return "";
    }
    
    switch (level) {
        case LOG_LEVEL_DEBUG: return "\033[36m"; // Cyan
        case LOG_LEVEL_INFO:  return "\033[32m"; // Green
        case LOG_LEVEL_WARN:  return "\033[33m"; // Yellow
        case LOG_LEVEL_ERROR: return "\033[31m"; // Red
        case LOG_LEVEL_FATAL: return "\033[35m"; // Magenta
        default: return "\033[0m"; // Reset
    }
}

void logger_log(log_level_t level, const char* file, int line, const char* format, ...) {
    // Check if we should log this message
    if (level < logger_config.min_level) {
        return;
    }
    
    // Get current time
    time_t now = time(NULL);
    struct tm* timeinfo = localtime(&now);
    
    // Print timestamp
    if (logger_config.enable_timestamp) {
        fprintf(logger_config.output_stream, "[%02d:%02d:%02d] ", 
                timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    }
    
    // Print log level with color
    if (logger_config.enable_level) {
        fprintf(logger_config.output_stream, "%s[%5s]\033[0m ", 
                logger_level_to_color(level), logger_level_to_string(level));
    }
    
    // Print file and line number
    if (logger_config.enable_file_line) {
        const char* filename = strrchr(file, '/');
        if (!filename) filename = file;
        else filename++;
        
        fprintf(logger_config.output_stream, "(%s:%d) ", filename, line);
    }
    
    // Print the message
    va_list args;
    va_start(args, format);
    vfprintf(logger_config.output_stream, format, args);
    va_end(args);
    
    // New line
    fprintf(logger_config.output_stream, "\n");
    
    // Flush for immediate output
    fflush(logger_config.output_stream);
}

void logger_hex_dump(const char* label, const void* data, size_t size) {
    if (logger_config.min_level > LOG_LEVEL_DEBUG) {
        return;
    }
    
    const unsigned char* bytes = (const unsigned char*)data;
    
    fprintf(logger_config.output_stream, "%s (%zu bytes):\n", label, size);
    
    for (size_t i = 0; i < size; i += 16) {
        // Print offset
        fprintf(logger_config.output_stream, "  %04zx: ", i);
        
        // Print hex
        for (size_t j = 0; j < 16; j++) {
            if (i + j < size) {
                fprintf(logger_config.output_stream, "%02x ", bytes[i + j]);
            } else {
                fprintf(logger_config.output_stream, "   ");
            }
            
            if (j == 7) {
                fprintf(logger_config.output_stream, " ");
            }
        }
        
        fprintf(logger_config.output_stream, " ");
        
        // Print ASCII
        for (size_t j = 0; j < 16 && i + j < size; j++) {
            unsigned char c = bytes[i + j];
            if (c >= 32 && c <= 126) {
                fprintf(logger_config.output_stream, "%c", c);
            } else {
                fprintf(logger_config.output_stream, ".");
            }
        }
        
        fprintf(logger_config.output_stream, "\n");
    }
    
    fflush(logger_config.output_stream);
}