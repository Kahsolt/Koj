#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"
#include "judge.h"
#include "utils.h"

#define VERSION 0x0100

#define DEFAULT_PORT    7000
#define DEFAULT_LOG_FN  "koj.log"
#define DEFAULT_PID_FN  "koj.pid"
#define DEFAULT_SOCK_FN "koj.sock"

void help() {
  printf("Koj Judged [version %d.%d]\n", VERSION >> 8, VERSION & 0x3);
  printf("Note: should either use TCP(-p) the default or UnixSock(-s/-S), not both.\n");
  printf("Usage: judged [args]...\n");
  printf("  -p PORT\tSet TCP port (default: %d)\n", DEFAULT_PORT);
  printf("  -S     \tUse default UnixSock %s, override TCP\n", DEFAULT_SOCK_FN);
  printf("  -s SOCK\tSet UnixSock file\n");
  printf("  -l LOG \tSet log filename (default: %s)\n", DEFAULT_LOG_FN);
  printf("  -r PID \tSet pid filename (default: %s)\n", DEFAULT_PID_FN);
  printf("  -t     \tRun local test then exit (make sure pwd=\"/judged/example\")\n");
  printf("  -h     \tShow this help then exit\n");

  exit(EXIT_SUCCESS);
}

int create_pid(char* pid_fn) {
  pid_fp = fopen(pid_fn, "rb");
  if(pid_fp != NULL) {
    fclose(pid_fp);
    LOG_WARN(log_fp, "force overwrite %s", pid_fn);
  }
  pid_fp = fopen(pid_fn, "wb");
  if(pid_fp == NULL) {
    LOG_ERROR(log_fp, "Error create pid file");
    return -1;
  } else {
    fprintf(pid_fp, "%d", getpid());
    fflush(pid_fp);
    fclose(pid_fp);
    return 0;
  }
}

int main(int argc, char *argv[]) {
  int port = DEFAULT_PORT;
  char *log_fn = DEFAULT_LOG_FN;
  char *pid_fn = DEFAULT_PID_FN;
  char *sock_fn = DEFAULT_SOCK_FN;
  int is_runtest = 0;
  int use_unixsock = 0;

  int i;
  for(i=1; i<argc; i++) {
    if (strlen(argv[i]) != 2 || argv[i][0] != '-'
        || argv[i][1] == 'h') {
      help();
    } else if(argv[i][1] == 't') {
      is_runtest = 1;
    } else if(argv[i][1] == 'p') {
      if(++i>=argc) help();
      port = atoi(argv[i]);
    } else if(argv[i][1] == 's') {
      if(++i>=argc) help();
      sock_fn = argv[i];
      use_unixsock = 1;
    } else if(argv[i][1] == 'S') {
      use_unixsock = 1;
    } else if(argv[i][1] == 'l') {
      if(++i>=argc) help();
      log_fn = argv[i];
    } else if(argv[i][1] == 'r') {
      if(++i>=argc) help();
      pid_fn = argv[i];
    } else help();
  }

  log_fp = log_open(log_fn);
  if(is_runtest) {
    judge_test();
    exit(EXIT_SUCCESS);
  } else if(create_pid(pid_fn) == 0) {
    if (use_unixsock)
      server_run_unixsock(sock_fn);
    else
      server_run_tcp(port);
  }
  log_close(log_fp);

  return 0;
}
