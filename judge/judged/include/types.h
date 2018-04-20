#ifndef JUDGED_TYPES_H
#define JUDGED_TYPES_H

#include <stdio.h>

#define EXEC_ARGC 32

typedef enum {
  // Common
  SUCCESS           = 0,
  FOPEN_FAILED      = -1,
  MALLOC_FAILED     = -2,
  // Socket
  SOCKET_ERROR      = -41,
  BIND_FAILED       = -42,
  LISTEN_FAILED     = -43,
  ACCEPT_FAILED     = -44,
  RECV_FAILED       = -45,
  SEND_FAILED       = -46,
  // Compiler
  BAD_SRC_FILE      = -11,
  COMPILE_FAILED    = -12,
  // Runner
  INVALID_CONFIG    = -21,
  FORK_FAILED       = -22,
  PTHREAD_FAILED    = -23,
  WAIT_FAILED       = -24,
  SETRLIMIT_FAILED  = -25,
  DUP2_FAILED       = -26,
  SECCOMP_FAILED    = -27,
  EXECVE_FAILED     = -28,
  CHILD_FAILED      = -29,
  // Comparor
  MISMATH           = -31,
  NOT_ALIKE         = -32,
} error_code;

typedef enum {
  ACCEPT,
  COMPILE_ERROR,
  RUNTIME_ERROR,
  TIME_LIMIT_EXCEEDED,
  MEMORY_LIMIT_EXCEEDED,
  OUTPUT_LIMIT_EXCEEDED,
  PRESENTATION_ERROR,
  WRONG_ANSWER,
  SYSTEM_ERROR,
} result_code;

static char* RESULT_CODE[] = {
    "AC", "CE", "RE", "TLE", "MLE", "OLE", "PE", "WA", "SE"
};

typedef struct {
  unsigned int limit_time_cpu;   /* ms, user_time + sys_time*/
  unsigned int limit_time_real;  /* ms, plus schedule and so on */
  unsigned int limit_memory;     /* KB */
  unsigned int limit_output;     /* KB */
  unsigned int limit_stack;      /* KB */
  unsigned int limit_process_number;
  char* work_dir;
  char* src_path;
  char* compile_cmd;
  char* exe_path;
  char* argv[EXEC_ARGC];
  char* envp[EXEC_ARGC];
  char* input_path;
  char* output_path;
  char* error_path;
  char* ans_path;
} task_t;

typedef struct {
  result_code result; /* enum: int */
  unsigned int time;        /* ms */
  unsigned int memory;      /* KB */
  unsigned int code_length; /* B */
} result_t;

/* global vars */
FILE* log_fp;
FILE* pid_fp;

#endif //JUDGED_TYPES_H