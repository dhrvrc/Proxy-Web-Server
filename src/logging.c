#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>

static FILE *g_log_file = NULL;
static LogLevel g_log_level = LOG_LEVEL_INFO;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

static const char* log_level_str(LogLevel level) {
    switch(level) {
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_INFO:  return "INFO";
        case LOG_LEVEL_WARN:  return "WARN";
        case LOG_LEVEL_ERROR: return "ERROR";
        default:              return "UNKNOWN";
    }
}

void init_logging(const char *log_file, LogLevel level) {
    pthread_mutex_lock(&log_mutex);
    g_log_level = level;
    if (log_file != NULL) {
        g_log_file = fopen(log_file, "a");
        if (!g_log_file) {
            // If opening the file fails, fallback to stdout.
            g_log_file = stdout;
        }
    } else {
        g_log_file = stdout;
    }
    pthread_mutex_unlock(&log_mutex);
}

void close_logging() {
    pthread_mutex_lock(&log_mutex);
    if (g_log_file && g_log_file != stdout) {
        fclose(g_log_file);
    }
    g_log_file = NULL;
    pthread_mutex_unlock(&log_mutex);
}

void log_message(LogLevel level, const char *format, ...) {
    if (level < g_log_level)
        return;

    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    char time_buf[20];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", local_time);

    pthread_mutex_lock(&log_mutex);
    va_list args;
    va_start(args, format);

    // Log to the configured log file.
    if (g_log_file) {
        fprintf(g_log_file, "[%s] %s: ", time_buf, log_level_str(level));
        va_list args_copy;
        va_copy(args_copy, args);
        vfprintf(g_log_file, format, args_copy);
        va_end(args_copy);
        fprintf(g_log_file, "\n");
        fflush(g_log_file);
    }

    // Also print to the CLI (stdout).
    fprintf(stdout, "[%s] %s: ", time_buf, log_level_str(level));
    vfprintf(stdout, format, args);
    fprintf(stdout, "\n");
    fflush(stdout);

    va_end(args);
    pthread_mutex_unlock(&log_mutex);
}
