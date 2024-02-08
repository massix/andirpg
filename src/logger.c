#include "logger.h"
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Logger *static_instance = nullptr;

struct Logger {
  FILE    *_file;
  char    *_file_path;
  LogLevel _min_level;
  uint64_t _logged_lines;
};

const char *log_level_to_string(LogLevel log_level) {
  switch (log_level) {
    case DEBUG:
      return "DEBUG";
    case INFO:
      return "INFO";
    case NOTICE:
      return "NOTICE";
    case WARNING:
      return "WARNING";
    case ERROR:
      return "ERROR";
    case CRITICAL:
      return "CRITICAL";
    default:
      return "UNKNOWN";
  }
}

Logger *logger_new(const char *file_path, LogLevel min_level) {
  if (static_instance != nullptr) {
    logger_free(static_instance);
  }

  static_instance = calloc(1, sizeof(Logger));
  static_instance->_file_path = strdup(file_path);
  static_instance->_file = fopen(static_instance->_file_path, "w");
  static_instance->_min_level = min_level;
  static_instance->_logged_lines = 0;

  if (static_instance->_file == nullptr) {
    fprintf(stderr, "Unable to open file '%s' for writing. Logs disabled!\n", static_instance->_file_path);
    free(static_instance->_file_path);
    free(static_instance);
    static_instance = nullptr;
  }

  return static_instance;
}

void logger_msg(Logger *logger, LogLevel log_level, const char *file, int line, const char *fmt, ...) {
  if (logger != nullptr && log_level >= logger->_min_level) {
    va_list variadic;
    va_start(variadic, fmt);
    char *custom_fmt = calloc(1024 + strlen(fmt), sizeof(char));
    sprintf(custom_fmt, "[%s:%d] [%s] %lu %s\n", file, line, log_level_to_string(log_level), logger->_logged_lines, fmt);
    vfprintf(logger->_file, custom_fmt, variadic);
    fflush(logger->_file);
    free(custom_fmt);
    va_end(variadic);
    logger->_logged_lines++;
  }
}

Logger *logger_instance() {
  return static_instance;
}

void logger_free(Logger *logger) {
  if (logger != nullptr) {
    free(logger->_file_path);

    fflush(logger->_file);
    fclose(logger->_file);

    free(logger);

    logger = nullptr;
    static_instance = nullptr;
  }
}

