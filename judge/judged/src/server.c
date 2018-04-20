#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "server.h"
#include "judge.h"
#include "utils.h"

void server_run_tcp(int port) {
  server_start();

  struct sockaddr_in server, client;
  int server_fd, client_fd;

  /* prepare server */
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    SERVER_EXIT(SOCKET_ERROR);
  int reuse = SO_REUSEADDR; /* allow address reuse */
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  int nodelay = 1;	        /* disable Nagle’s Algorithm, no packet merge */
  setsockopt(server_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));
  int timeout = 3000;       /* timout for send()/recv() */
  setsockopt(server_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
  setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
 
  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(server_fd, (struct sockaddr*) &server, sizeof(server)) == -1)
    SERVER_EXIT(BIND_FAILED);
  if(listen(server_fd, BACKLOG) == -1)
    SERVER_EXIT(LISTEN_FAILED);

  /* wait for client */
  socklen_t socket_len = sizeof(client);
  static char task_buf[TASK_JSON_BUF_SIZE];
  task_t* task = malloc(sizeof(task_t));
  if (task == NULL) SERVER_EXIT(MALLOC_FAILED);
  result_t* result = malloc(sizeof(result_t));
  if (result == NULL) SERVER_EXIT(MALLOC_FAILED);
  init_result(result);
  char* result_json = malloc(RESULT_JSON_BUF_SIZE * sizeof(char));
  if (result_json == NULL) SERVER_EXIT(MALLOC_FAILED);
  /* handle multiple connections */
  while (1) {
    if((client_fd = accept(server_fd,(struct sockaddr*) &client, &socket_len)) == -1) {
      LOG_ERROR(log_fp, "Error on accept(), System error: %s", strerror(errno));
      continue;
    }
    // LOG_DEBUG(log_fp, "client from <%s:%d> ", inet_ntoa(client.sin_addr), htons(client.sin_port));

    /* handle single request (TODO: handle multiple requests via single connection) */
    memset(task_buf, 0, TASK_JSON_BUF_SIZE);
    int count = recv(client_fd, task_buf, TASK_JSON_BUF_SIZE, 0);
    if(count <= 0) {
      LOG_WARN(log_fp, "bad data on recv()");
    } else {  /* THE BUSINESS */
      parse_task(task_buf, task);
      init_result(result);
      judge(task, result);
      stringify_result(result, result_json);
      send(client_fd, result_json, strlen(result_json), 0);
    }
    close(client_fd);
  }
}

void server_run_unixsock(char *sock_fn) {
  server_start();

  struct sockaddr_un server, client;
  int server_fd, client_fd;

  /* prepare server */
  if((server_fd = socket(PF_UNIX, SOCK_STREAM, 0)) == -1)
    SERVER_EXIT(SOCKET_ERROR);
  server.sun_family = AF_UNIX;
  strcpy(server.sun_path, sock_fn);
  unlink(sock_fn);
  if((bind(server_fd,(struct sockaddr*) &server, sizeof(server))) == -1)
    SERVER_EXIT(BIND_FAILED);
  if(listen(server_fd, BACKLOG) == -1)
    SERVER_EXIT(LISTEN_FAILED)

  /* wait for client */
  socklen_t socket_len = sizeof(client);
  static char task_buf[TASK_JSON_BUF_SIZE];
  task_t* task = malloc(sizeof(task_t));
  if (task == NULL) SERVER_EXIT(MALLOC_FAILED);
  result_t* result = malloc(sizeof(result_t));
  if (result == NULL) SERVER_EXIT(MALLOC_FAILED);
  init_result(result);
  char* result_json = malloc(RESULT_JSON_BUF_SIZE * sizeof(char));
  if (result_json == NULL) SERVER_EXIT(MALLOC_FAILED);
  /* handle multiple connections */
  while (1) {
    if((client_fd = accept(server_fd,(struct sockaddr*) &client, &socket_len)) == -1) {
      LOG_ERROR(log_fp, "Error on accept(), System error: %s", strerror(errno));
      continue;
    }
    // LOG_DEBUG(log_fp, "client through %s ", server.sun_path);

    memset(task_buf, 0, TASK_JSON_BUF_SIZE);
    int count = read(client_fd, task_buf, TASK_JSON_BUF_SIZE);
    if(count <= 0) {
      LOG_WARN(log_fp, "bad data on read()");
    } else {
      parse_task(task_buf, task);
      init_result(result);
      judge(task, result);
      stringify_result(result, result_json);
      write(client_fd, result_json, (unsigned int)strlen(result_json));
    }
    close(client_fd);
  }
}

/* private util */
void server_start() {
  LOG_INFO(log_fp, "Server starting.");
  signal(SIGINT,  server_stop);
  signal(SIGABRT, server_stop);
  signal(SIGHUP,  server_stop);
  signal(SIGQUIT, server_stop);
  signal(SIGTERM, server_stop);
  signal(SIGUSR2, server_stop);
}

void server_stop(int signal) {
  LOG_INFO(log_fp, "Server shutting down on signal %d.", signal);
  log_close(log_fp);
  exit(EXIT_SUCCESS);
}