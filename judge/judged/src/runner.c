#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <sched.h>
#include <seccomp.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <types.h>

#include "runner.h"
#include "utils.h"

/* private util*/
error_code run(task_t *task, result_t *result) {
  if (task->exe_path == NULL || task->output_path == NULL) {
    if(task->exe_path     == NULL) LOG_ERROR(log_fp, "Bad task config, missing exe_path");
    if(task->output_path  == NULL) LOG_ERROR(log_fp, "Bad task config, missing output_path");
    return INVALID_CONFIG;
  }

  struct timeval start_time, end_time;
  gettimeofday(&start_time, NULL);

  pid_t child_pid = fork();
  if (child_pid < 0) {
    LOG_ERROR(log_fp, "Error on fork()");
    return(FORK_FAILED);
  } else if (child_pid == 0) {
    child_process(task);
  } else if (child_pid > 0) {
    /* bind child a killer */
    pthread_t tid = 0;
    if (task->limit_time_real != UNLIMITED) {
      killer_task_t killer_task = {
          .pid = child_pid,
          .timeout = task->limit_time_real
      };
      if (pthread_create(&tid, NULL, kill_timeout, (void*)&killer_task) != 0) {
        kill_pid(child_pid);
        LOG_FATAL(log_fp, "Error on pthread_create()");
        return PTHREAD_FAILED;
      }
    }

    /* wait for child finish */
    int status;
    struct rusage resource_usage;
    if (wait4(child_pid, &status, WUNTRACED, &resource_usage) == -1) {
      kill_pid(child_pid);
      LOG_ERROR(log_fp, "Error on wait4()");
      return WAIT_FAILED;
    }
    gettimeofday(&end_time, NULL);
    if (tid != 0) pthread_cancel(tid);

    /* record resource usage */
    unsigned int real_time = (unsigned int) (end_time.tv_sec * 1000 + end_time.tv_usec / 1000
                                               - start_time.tv_sec * 1000 - start_time.tv_usec / 1000);
    unsigned int user_time = (unsigned int) (resource_usage.ru_stime.tv_sec * 1000
                                               + resource_usage.ru_stime.tv_usec / 1000);
    unsigned int sys_time = (unsigned int) (resource_usage.ru_utime.tv_sec * 1000
                                              + resource_usage.ru_utime.tv_usec / 1000);
    unsigned int cpu_time = user_time + sys_time;
    result->time = cpu_time;  /* ms */
    unsigned int memory = resource_usage.ru_maxrss;
    result->memory = memory;  /* KB */
    unsigned int output = 0;
    FILE* output_fp = fopen(task->ans_path, "rb");
    if(output_fp != NULL) {
      fseek(output_fp, 0, SEEK_END);
      output = (unsigned int) ftell(output_fp) >> 10; /* KB */
      fclose(output_fp);
    }
  	LOG_DEBUG(log_fp, "Resource Usage: real_time=%u cpu_time=%u(user=%u sys=%u) memory=%u output=%u",
              real_time, cpu_time, user_time, sys_time, memory, output);

    /* decide result code */
    if (WIFSIGNALED(status) != 0) {   /* the signal that child got and stopped */
      int signal = WTERMSIG(status);
      if(signal == SIGKILL || signal == SIGXCPU)  /* timeout killer or RLIMIT_CPU */
        result->result = TIME_LIMIT_EXCEEDED;
      else if(signal == SIGXFSZ)       /* RLIMIT_FSIZE */
        result->result = OUTPUT_LIMIT_EXCEEDED;
      else if(signal == SIGSYS)       /* seccomp (bad syscall) */
        result->result = RUNTIME_ERROR;
      else if(signal == SIGSEGV) {    /* RLIMIT_STACK or RLIMIT_AS (any memory issue) */
        if (task->limit_memory != UNLIMITED && memory > task->limit_memory)
          result->result = MEMORY_LIMIT_EXCEEDED;
        else
          result->result = RUNTIME_ERROR;
      } else {
        if(signal != SIGUSR1) LOG_WARN(log_fp, "child got an unknown signal %d", signal);
        return CHILD_FAILED;
      }
    } else {
      if ((task->limit_time_cpu != UNLIMITED && cpu_time > task->limit_time_cpu)
               || (task->limit_time_real != UNLIMITED && real_time > task->limit_time_real))
        result->result = TIME_LIMIT_EXCEEDED;
      else if (task->limit_memory != UNLIMITED && memory > task->limit_memory)
        result->result = MEMORY_LIMIT_EXCEEDED;
      else if (task->limit_output != UNLIMITED && output > task->limit_output)
        result->result = OUTPUT_LIMIT_EXCEEDED;
    }
  }

  return SUCCESS;
}

int seccomp_rules(task_t *task) {
  int syscalls_length, i;

  scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW);
  if (ctx == NULL) {
    LOG_ERROR(log_fp, "Error on seccomp_init()");
    return SECCOMP_FAILED;
  }

  /* if use seccomp_init(SCMP_ACT_ALLOW) then use this blacklist */
  int syscalls_blacklist[] = {SCMP_SYS(clone), SCMP_SYS(kill),
                              SCMP_SYS(fork), SCMP_SYS(vfork)};
  syscalls_length = sizeof(syscalls_blacklist) / sizeof(int);
  for (i = 0; i < syscalls_length; i++)
    if (seccomp_rule_add(ctx, SCMP_ACT_KILL, syscalls_blacklist[i], 0) != 0)
      LOG_DEBUG(log_fp, "Error on seccomp_rule_add() for blacklist[%d]", i);

  /* if use seccomp_init(SCMP_ACT_KILL) then use this whitelist
  int syscalls_whitelist[] = {SCMP_SYS(brk), SCMP_SYS(mmap), SCMP_SYS(mprotect), SCMP_SYS(munmap),
                              SCMP_SYS(uname), SCMP_SYS(sysinfo), SCMP_SYS(arch_prctl),
                              SCMP_SYS(access), SCMP_SYS(exit_group),
                              SCMP_SYS(fstat), SCMP_SYS(readlink), SCMP_SYS(read), SCMP_SYS(write),
                              SCMP_SYS(close), SCMP_SYS(writev), SCMP_SYS(lseek)};
  syscalls_length = sizeof(syscalls_whitelist) / sizeof(int);
  for (i = 0; i < syscalls_length; i++)
    if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, syscalls_blacklist[i], 0) != 0)
      LOG_DEBUG(log_fp, "Error on seccomp_rule_add() for whitelist[%d]", i);
  */

  if (seccomp_rule_add(ctx, SCMP_ACT_ERRNO(EACCES), SCMP_SYS(socket), 0) != 0
      // allow on execve
      // || seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(execve), 1, SCMP_A0(SCMP_CMP_NE, (scmp_datum_t) (task->exe_path))) != 0
      // kill on file write
      || seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(open),   1, SCMP_CMP(0, SCMP_CMP_EQ, O_WRONLY)) != 0
      || seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(open),   1, SCMP_CMP(0, SCMP_CMP_EQ, O_RDWR))   != 0
      || seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(openat), 1, SCMP_CMP(0, SCMP_CMP_EQ, O_WRONLY)) != 0
      || seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(openat), 1, SCMP_CMP(0, SCMP_CMP_EQ, O_RDWR))   != 0
     )
    LOG_DEBUG(log_fp, "Error on seccomp_rule_add() for misc extras rules");

  if (seccomp_load(ctx) != 0) {
    LOG_ERROR(log_fp, "Error on seccomp_load()");
    return SECCOMP_FAILED;
  }
  seccomp_release(ctx);

  return SUCCESS;
}

int kill_pid(pid_t pid) {
  LOG_WARN(log_fp, "kill_pid called");
  return kill(pid, SIGKILL);
}

void* kill_timeout(void* task) {
  pid_t pid = ((killer_task_t*)task)->pid;
  int timeout = ((killer_task_t*)task)->timeout;

  pthread_detach(pthread_self()); // assume success ( == 0)
  usleep((useconds_t)(timeout * 1000 * 1.1)); // milli to micro
  kill_pid(pid);

  return NULL;
}

void child_process(task_t* task) {
  FILE *input_file = NULL, *output_file = NULL, *error_file = NULL;

  /* set rlimit */
  if (task->limit_time_cpu != UNLIMITED) {
    struct rlimit limit_time_cpu;
    limit_time_cpu.rlim_cur = limit_time_cpu.rlim_max = (rlim_t) (task->limit_time_cpu / 1000); // ms to s
    if (setrlimit(RLIMIT_CPU, &limit_time_cpu) != 0)
    RUNNER_CHILD_EXIT(SETRLIMIT_FAILED, "cpu");
  }
  if (task->limit_stack != UNLIMITED) {
    struct rlimit limit_stack;
    limit_stack.rlim_cur = limit_stack.rlim_max = (rlim_t) task->limit_stack * 1024; // KB to B
    if (setrlimit(RLIMIT_STACK, &limit_stack) != 0)
    RUNNER_CHILD_EXIT(SETRLIMIT_FAILED, "stack");
  }
  if (task->limit_memory != UNLIMITED) {
    struct rlimit limit_memory;
    limit_memory.rlim_cur = limit_memory.rlim_max = (rlim_t) task->limit_memory * 1024; // KB to B
    if (setrlimit(RLIMIT_AS, &limit_memory) != 0)
    RUNNER_CHILD_EXIT(SETRLIMIT_FAILED, "memory as");
    if (setrlimit(RLIMIT_DATA, &limit_memory) != 0)
    RUNNER_CHILD_EXIT(SETRLIMIT_FAILED, "memory data");
    // if (setrlimit(RLIMIT_MEMLOCK, &limit_memory) != 0)
    // RUNNER_CHILD_EXIT(SETRLIMIT_FAILED, "memory lock");
  }
  if (task->limit_output != UNLIMITED) {
    struct rlimit limit_output;
    limit_output.rlim_cur = limit_output.rlim_max = (rlim_t) task->limit_output * 1024; // KB to B
    if (setrlimit(RLIMIT_FSIZE, &limit_output) != 0)
    RUNNER_CHILD_EXIT(SETRLIMIT_FAILED, "output");
  }
  if (task->limit_process_number != UNLIMITED) {
    struct rlimit limit_process_number;
    limit_process_number.rlim_cur = limit_process_number.rlim_max = (rlim_t) task->limit_process_number;
    if (setrlimit(RLIMIT_NPROC, &limit_process_number) != 0)
    RUNNER_CHILD_EXIT(SETRLIMIT_FAILED, "processes");
  }
  struct rlimit limit_core_dump;
  limit_core_dump.rlim_cur = limit_core_dump.rlim_max = (rlim_t) 0;
  if (setrlimit(RLIMIT_CORE, &limit_core_dump) != 0)
  RUNNER_CHILD_EXIT(SETRLIMIT_FAILED, "coredump");

  /* load seccomp filters
  if (seccomp_rules(task) != 0)
    LOG_WARN(log_fp, "cannot load seccomp rules");
  */

  /* open files and redirect */
  if (strlen(task->input_path) != 0) {
    input_file = fopen(task->input_path, "rb");
    if (input_file == NULL || dup2(fileno(input_file), fileno(stdin)) == -1)
    RUNNER_CHILD_EXIT(DUP2_FAILED, task->input_path);
  }
  if (strlen(task->ans_path) != 0) {
    output_file = fopen(task->ans_path, "wb");
    if (output_file == NULL || dup2(fileno(output_file), fileno(stdout)) == -1)
    RUNNER_CHILD_EXIT(DUP2_FAILED, task->output_path);
  }
  if (strlen(task->error_path) != 0) {
    if (task->output_path != 0 && strcmp(task->output_path, task->error_path) == 0) {
      error_file = output_file;
    } else {
      error_file = fopen(task->error_path, "wb");
      if (error_file == NULL || dup2(fileno(error_file), fileno(stderr)) == -1)
      RUNNER_CHILD_EXIT(DUP2_FAILED, task->error_path);
    }
  }
  
  /* exec task */
  execve(task->exe_path, task->argv, task->envp);
  RUNNER_CHILD_EXIT(EXECVE_FAILED, task->exe_path);
}

