#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <sys/file.h>

#include "logger.h"

FILE* log_open(const char* log_fn) {
  FILE* log_fp = fopen(log_fn, "a");
#ifdef LOG_NOFAIL
  if (log_fp == NULL)
    fprintf(stderr, "Error open log file %s", log_fn);
#endif
  return log_fp;
}

void log_close(FILE *log_fp) {
  if (log_fp != NULL)
    fclose(log_fp);
}

void log_write(int level, const char* __file__, int __line__,
               FILE* log_fp, const char* fmt, ...) {
  if (log_fp == NULL || level < LOG_THRESHOLD) return;

  static char* LOG_LEVEL_NOTE[] = { "DEBUG", "INFO ", "WARN ", "ERROR", "FATAL" };
  static char log_buf[LOG_BUFFER_SIZE];
  static char buffer[LOG_BUFFER_SIZE];
  static char timestamp[21];

  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buffer, LOG_BUFFER_SIZE, fmt, ap);
  va_end(ap);

  time_t now = time(NULL);
  strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
  int count = snprintf(log_buf, LOG_BUFFER_SIZE, "%s [%s] <%s:%d> %s\n",
                       LOG_LEVEL_NOTE[level], timestamp, __file__, __line__, buffer);
  if (level >= LOG_LEVEL_INFO) fprintf(stdout, "%s", log_buf);

  int log_fd = fileno(log_fp);
  if (flock(log_fd, LOCK_EX) == 0 && write(log_fd, log_buf, (unsigned int)(count)) != -1)
    flock(log_fd, LOCK_UN);
  else
    fprintf(log_fp, "%s", log_buf);
}

void log_test() {
  FILE* log_fp = log_open("test.log");
  LOG_DEBUG(log_fp, "1st is log without fmt");
  LOG_DEBUG(log_fp, "%dnd is log without %s", 2, "fmt");

  LOG_DEBUG(log_fp, "%s", "this is debug");
  LOG_INFO (log_fp, "%s", "this is info");
  LOG_WARN (log_fp, "%s", "this is warn");
  LOG_ERROR(log_fp, "%s", "this is error");
  LOG_FATAL(log_fp, "%s", "this is fatal");
  log_close(log_fp);
}