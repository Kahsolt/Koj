#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "utils.h"

void print_task(task_t *task) {
  printf("{\n"
             "\t\"limit_time_cpu\": %u,\n"
             "\t\"limit_time_real\": %u,\n"
             "\t\"limit_memory\": %u,\n"
             "\t\"limit_output\": %u,\n"
             "\t\"limit_stack\": %u,\n"
             "\t\"limit_process_number\": %u,\n"
             "\t\"work_dir\": \"%s\",\n"
             "\t\"src_path\": \"%s\",\n"
             "\t\"compile_cmd\": \"%s\",\n"
             "\t\"exe_path\": \"%s\",\n",
         task->limit_time_cpu,
         task->limit_time_real,
         task->limit_memory,
         task->limit_output,
         task->limit_stack,
         task->limit_process_number,
         task->work_dir,
         task->src_path,
         task->compile_cmd,
         task->exe_path);

  int idx;

  printf("\t\"argv\": {");
  idx = 0;
  while (task->argv[idx] != NULL)
    printf("\"%s\", ", task->argv[idx++]);
  printf("NULL},\n");

  printf("\t\"envp\": {");
  idx = 0;
  while (task->envp[idx] != NULL)
    printf("\"%s\", ", task->envp[idx++]);
  printf("NULL},\n");

  printf("\t\"input_path\": \"%s\",\n"
             "\t\"output_path\": \"%s\",\n"
             "\t\"error_path\": \"%s\",\n"
             "\t\"ans_path\": \"%s\",\n"
             "}\n",
         task->input_path,
         task->output_path,
         task->error_path,
         task->ans_path);
}

void print_result(result_t *result) {
  printf("{\n"
             "\t\"result\": \"%s\",\n"
             "\t\"time\": %d,\n"
             "\t\"memory\": %d,\n"
             "\t\"code_length\": %d\n"
             "}\n",
         RESULT_CODE[result->result],
         result->time,
         result->memory,
         result->code_length);
}

void free_task(task_t *task) {
  if(task == NULL) return;

  int idx;

  if(task->work_dir     != NULL)  free(task->work_dir);
  if(task->src_path     != NULL)  free(task->src_path);
  if(task->compile_cmd  != NULL)  free(task->compile_cmd);
  if(task->exe_path     != NULL)  free(task->exe_path);
  idx = 0; while (task->argv[idx] != NULL) {
    free(task->argv[idx]);
    idx++;
  }
  idx = 0; while (task->envp[idx] != NULL) {
    free(task->envp[idx]);
    idx++;
  }
  if(task->input_path   != NULL)  free(task->input_path);
  if(task->output_path  != NULL)  free(task->output_path);
  if(task->error_path   != NULL)  free(task->error_path);
  if(task->ans_path     != NULL)  free(task->ans_path);

  free(task);
}

void init_result(result_t *result) {
  result->time = result->memory = 0;
  result->code_length = 0;
  result->result = SYSTEM_ERROR;
}

void parse_task(const char* task_json, task_t* task) {  // FIXME: no format error handle
  unsigned int nums[6];
  char tmp[256], *paths[8], *q;
  const char *p=task_json;
  int i;

  for(i=0; i<6; i++) {
    q = tmp;
    while(*p++ != ':');
    while(*p != ',') *q++ = *p++;
    *q = '\0';
    nums[i] = strtoul(tmp, NULL, 0);
  }
  task->limit_time_cpu        = nums[0];
  task->limit_time_real       = nums[1];
  task->limit_memory          = nums[2];
  task->limit_output          = nums[3];
  task->limit_stack           = nums[4];
  task->limit_process_number  = nums[5];

  for(i=0; i<4; i++) {
    q = tmp;
    while(*p++ != ':'); p++;
    while(*p != '"') *q++ = *p++;
    *q = '\0';
    paths[i] = malloc((strlen(tmp) + 1) * sizeof(char));
    strcpy(paths[i], tmp);
  }
  task->work_dir    = paths[0];
  task->src_path    = paths[1];
  task->compile_cmd = paths[2];
  task->exe_path    = paths[3];

  while(*p++!='['); p++;
  for(i=0; *p!=']'; i++) {
    q = tmp;
    while(*p != '"') *q++ = *p++;
    *q = '\0'; p++;
    task->argv[i] = malloc((strlen(tmp) + 1) * sizeof(char));
    strcpy(task->argv[i], tmp);
    while(*p != '"') {
      if(*p == ']') break;
      p++;
    }
    if(*p == '"') p++;
  }
  task->argv[i] = NULL;

  while(*p++ != '['); p++;
  for(i=0; *p != ']'; i++){
    q = tmp;
    while(*p != '"') *q++ = *p++;
    *q = '\0'; p++;
    task->envp[i] = malloc((strlen(tmp) + 1) * sizeof(char));
    strcpy(task->envp[i], tmp);
    while(*p != '"'){
      if(*p == ']')break;
      p++;
    }
    if(*p == '"') p++;
  }
  task->envp[i] = NULL;

  for(i=4; i<8; i++) {
    q = tmp;
    while(*p++ != ':'); p++;
    while(*p != '"') *q++ = *p++;
    *q ='\0';
    paths[i] = malloc((strlen(tmp) + 1) * sizeof(char));
    strcpy(paths[i], tmp);
  }
  task->input_path  = paths[4];
  task->output_path = paths[5];
  task->error_path  = paths[6];
  task->ans_path    = paths[7];
}

void stringify_result(result_t* result, char* result_json) {  // FIXME: no format error handle
  sprintf(result_json, "{\"result\":\"%s\",\"time\":%u,\"memory\":%u,\"code_length\":%u}",
          RESULT_CODE[result->result], result->time, result->memory, result->code_length);
}

void json_util_test() {
  char* task_json = "{"
      "\"limit_time_cpu\":1000,"
      "\"limit_time_real\":3000,"
      "\"limit_memory\":4096,"
      "\"limit_output\":4096,"
      "\"limit_stack\":4096,"
      "\"limit_process_number\":10,"
      "\"work_dir\":\"example\","
      "\"src_path\":\"main.c\","
      "\"compile_cmd\":\"gcc main.c -o main\","
      "\"exe_path\":\"./main\","
      "\"argv\":[\"./main\",\"-rm\",\"233\",null],"
      "\"envp\":[\"PATH=/bin:/usr/bin\",\"USER=nobody\",null],"
      "\"input_path\":\"1.in\","
      "\"output_path\":\"1.out\","
      "\"error_path\":\"/dev/null\","
      "\"ans_path\":\"/data/1/1.out\""
      "}";
  printf("task_json:\n%s\n", task_json);


  task_t* task = malloc(sizeof(task_t));
  parse_task(task_json, task);
  print_task(task);
  free_task(task);

  result_t result = {
      .result = ACCEPT,
      .time = 233,
      .memory = 500,
      .code_length = 666,
  };

  print_result(&result);
  char* result_json = malloc(RESULT_JSON_BUF_SIZE * sizeof(char));
  stringify_result(&result, result_json);
  printf("result_json:\n%s\n", result_json);
  free(result_json);
}