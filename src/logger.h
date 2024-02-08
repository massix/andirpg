#ifndef __LOGGER__H__
#define __LOGGER__H__

typedef enum LogLevel { DEBUG = 0, INFO, NOTICE, WARNING, ERROR, CRITICAL } LogLevel;
typedef struct Logger Logger;

#define LOG_DEBUG(fmt, ...)    logger_msg(logger_instance(), DEBUG, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define LOG_INFO(fmt, ...)     logger_msg(logger_instance(), INFO, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define LOG_NOTICE(fmt, ...)   logger_msg(logger_instance(), NOTICE, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define LOG_WARNING(fmt, ...)  logger_msg(logger_instance(), WARNING, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define LOG_ERROR(fmt, ...)    logger_msg(logger_instance(), ERROR, __FILE__, __LINE__, fmt, __VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) logger_msg(logger_instance(), CRITICAL, __FILE__, __LINE__, fmt, __VA_ARGS__)

Logger *logger_new(const char *file_path, LogLevel);
Logger *logger_instance();
void    logger_free(Logger *);

void logger_msg(Logger *, LogLevel, const char *filename, int line, const char *, ...);

#endif /* ifndef __LOGGER__H__ */

