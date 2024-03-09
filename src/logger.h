// AndiRPG - Name not final
// Copyright © 2024 Massimo Gengarelli
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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

