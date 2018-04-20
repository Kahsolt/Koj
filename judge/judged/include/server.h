#ifndef JUDGED_SERVER_H
#define JUDGED_SERVER_H

#include <stdlib.h>
#include <errno.h>

#define BACKLOG 1   /* for listen() :backlog: system queue size */

#define SERVER_EXIT(error_code) \
  { \
    LOG_FATAL(log_fp, "System error: %s, Internal error: "#error_code, strerror(errno)); \
    close(server_fd); \
    exit(EXIT_FAILURE); \
  }

void server_run_tcp(int port);
void server_run_unixsock(char *sock_fn);

/* private util */
void server_start();
void server_stop(int signal);

#endif //JUDGED_SERVER_H