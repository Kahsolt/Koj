#ifndef JUDGED_RUNNER_H
#define JUDGED_RUNNER_H

#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>

#include "types.h"

#define UNLIMITED 0

#define RUNNER_CHILD_EXIT(error_code, detail) \
  { \
    LOG_ERROR(log_fp, "System error: %s, Internal error: "#error_code" on %s", strerror(errno), detail); \
    if(input_file  != NULL) fclose(input_file); \
    if(output_file != NULL) fclose(output_file); \
    if(error_file  != NULL) fclose(error_file); \
    raise(SIGUSR1); \
  }

typedef struct {
  pid_t pid;
  unsigned int timeout;
} killer_task_t;

error_code run(task_t *task, result_t *result);

/* private util */
void child_process(task_t* task);
int seccomp_rules(task_t *task);
int kill_pid(pid_t pid);
void* kill_timeout(void* task);  /* timeout in millisecond */

#endif //JUDGED_RUNNER_H