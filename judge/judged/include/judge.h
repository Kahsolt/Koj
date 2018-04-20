#ifndef JUDGED_JUDGE_H
#define JUDGED_JUDGE_H

#include "types.h"

void judge(task_t* task, result_t* result);
void judge_test();

/* ffi interface */
char* judge_json_ret(char* task_json, char* log_fn);

#endif //JUDGED_JUDGE_H