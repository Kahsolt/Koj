#ifndef JUDGED_UTILS_H
#define JUDGED_UTILS_H

#include "types.h"
#include "logger.h"

#define PATH_BUF_SIZE         128
#define TASK_JSON_BUF_SIZE    1024
#define RESULT_JSON_BUF_SIZE  128

void print_task(task_t *task);
void print_result(result_t *result);
void free_task(task_t *task);
void init_result(result_t *result);
void parse_task(const char* task_json, task_t* task);
void stringify_result(result_t* result, char* result_json);
void json_util_test();

#endif //JUDGED_UTILS_H