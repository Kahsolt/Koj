#include <unistd.h>

#include "judge.h"
#include "compiler.h"
#include "runner.h"
#include "comparor.h"
#include "utils.h"

void judge(task_t* task, result_t* result) {
  if(chdir(task->work_dir) != 0) {
    char pwd[PATH_BUF_SIZE]; getcwd(pwd, PATH_BUF_SIZE);
    LOG_ERROR(log_fp, "Error on chdir() to %s/%s", pwd, task->work_dir);
    return;
  }
  if (compile(task, result) == SUCCESS
      && run(task, result) == SUCCESS
      && result->result == SYSTEM_ERROR) /* SYSTEM_ERROR is default if unchanged */
    compare(task, result);
}

void judge_test_case_limit(
    unsigned long limit_time_cpu,
    unsigned long limit_time_real,
    unsigned long limit_memory,
    unsigned long limit_output,
    unsigned long limit_stack
) {
  task_t task={
      .limit_time_cpu=limit_time_cpu,
      .limit_time_real=limit_time_real,
      .limit_memory=limit_memory,
      .limit_output=limit_output,
      .limit_stack=limit_stack,
      .limit_process_number=10,
      .work_dir=".",		/* pwd=example */
      .src_path="main.c",
      .compile_cmd="gcc -ansi main.c -o main",
      .exe_path="main",
      .argv={"main", NULL},
      .envp={"PATH=/bin:/usr/bin", NULL},
      .input_path="1.in",
      .output_path="1.out",
      .error_path="/dev/null",
      .ans_path="1.inout",
  };

  result_t* result = malloc(sizeof(result));
  init_result(result);
  // chdir(task.work_dir);
  if(compile(&task, result) == SUCCESS
     && run(&task, result) == SUCCESS
     && result->result == SYSTEM_ERROR) /* SYSTEM_ERROR is the default unchanged value */
    compare(&task, result);

  printf("result=%s\ttime=%ums\tmemory=%uKB\tcode_length=%uB\n",
         RESULT_CODE[result->result], result->time, result->memory, result->code_length);
  free(result);
}
void judge_test() {
  printf("=== Test Integrity ===\n");
  judge_test_case_limit(1000, 3000, 512 * 1024, 4 * 1024, 8 * 1024);  // AC
  judge_test_case_limit(1,    3000, 512 * 1024, 4 * 1024, 8 * 1024);  // TLE
  judge_test_case_limit(1000, 1,    512 * 1024, 4 * 1024, 8 * 1024);  // TLE
  judge_test_case_limit(1000, 3000, 1,          4 * 1024, 8 * 1024);  // MLE
  judge_test_case_limit(1000, 3000, 512 * 1024, 1,        8 * 1024);  // OLE
  judge_test_case_limit(1000, 3000, 512 * 1024, 4 * 1024, 1       );  // RE
  printf("======= Finish =======\n");


  task_t task={
      .limit_time_cpu=1000,
      .limit_time_real=3000,
      .limit_memory=1048576,
      .limit_output=65536,
      .limit_stack=8192,
      .limit_process_number=100,
      .work_dir=".",
      .src_path="", // "main.c"
      .compile_cmd="",  // "gcc -ansi main.c -o main"
      .exe_path="main",
      .argv={"main", NULL},
      .envp={"PATH=/bin:/usr/bin", NULL},
      .input_path="1.in",
      .output_path="",  // "1.out"
      .error_path="/dev/null",
      .ans_path="1.inout",
  };
  result_t result;
  int i = 100;
  printf("=== Run Native C ===\n");
  system("date");
  while(i--) {
    init_result(&result);
    judge(&task, &result);
  }
  system("date");
  printf("======= Finish =======\n");

  char* task_json = "{\"limit_time_cpu\":1000,"
      "\"limit_time_real\":3000,"
      "\"limit_memory\":1048576,"
      "\"limit_output\":65536,"
      "\"limit_stack\":8192,"
      "\"limit_process_number\":100,"
      "\"work_dir\":\".\","
      "\"src_path\":\"\","
      "\"compile_cmd\":\"\","
      "\"exe_path\":\"./main\","
      "\"argv\":[\"./main\",null],"
      "\"envp\":[\"PATH=/bin:/usr/bin\",null],"
      "\"input_path\":\"1.in\","
      "\"output_path\":\"\","
      "\"error_path\":\"/dev/null\","
      "\"ans_path\":\"1.inout\""
      "}";
  char* result_json;
  i = 100;
  printf("=== Run via Json ====\n");
  system("date");
  while(i--) {
    result_json = judge_json_ret(task_json, NULL);
    // free(result_json);
  }
  system("date");
  printf("======= Finish =======\n");
}

/* ffi interface */
char* judge_json_ret(char* task_json, char* log_fn) {
  log_fp = log_open(log_fn);

  task_t task;
  result_t result;
  char* result_json = malloc(RESULT_JSON_BUF_SIZE * sizeof(char));
  if(result_json == NULL) {
    LOG_ERROR(log_fp, "Error on malloc() for char* result_json");
    return NULL;
  }

  parse_task(task_json, &task);
  init_result(&result);
  judge(&task, &result);
  stringify_result(&result, result_json);
  log_close(log_fp);

  return result_json;
}
