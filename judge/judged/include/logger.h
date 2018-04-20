#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO  1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_ERROR 3
#define LOG_LEVEL_FATAL 4

/* config this as needed */
#undef  LOG_NOFAIL
#define LOG_THRESHOLD   LOG_LEVEL_INFO
#define LOG_BUFFER_SIZE 8192

FILE* log_open(const char* log_fn);
void log_close(FILE* log_fp);
void log_write(int level, const char* __file__, int __line__,
               FILE* log_fp, const char* fmt, ...);
void log_test();

#define LOG_DEBUG(log_fp, ap...) log_write(LOG_LEVEL_DEBUG, __FILE__, __LINE__, log_fp, ##ap)
#define LOG_INFO( log_fp, ap...) log_write(LOG_LEVEL_INFO , __FILE__, __LINE__, log_fp, ##ap)
#define LOG_WARN( log_fp, ap...) log_write(LOG_LEVEL_WARN , __FILE__, __LINE__, log_fp, ##ap)
#define LOG_ERROR(log_fp, ap...) log_write(LOG_LEVEL_ERROR, __FILE__, __LINE__, log_fp, ##ap)
#define LOG_FATAL(log_fp, ap...) log_write(LOG_LEVEL_FATAL, __FILE__, __LINE__, log_fp, ##ap)

#endif  // LOGGER_H