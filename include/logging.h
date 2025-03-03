#ifndef LOGGING_H
#define LOGGING_H

typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
} LogLevel;

/**
 * Initializes the logging system.
 *
 * @param log_file The file path to write logs to. If NULL, logs are printed to stdout.
 * @param level The minimum log level to output.
 */
void init_logging(const char *log_file, LogLevel level);

/**
 * Closes the logging system and frees any resources.
 */
void close_logging();

/**
 * Writes a log message with the given log level.
 *
 * @param level The log level for the message.
 * @param format printf-style format string.
 * @param ... Arguments for the format string.
 */
void log_message(LogLevel level, const char *format, ...);

#endif
