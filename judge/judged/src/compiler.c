#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "utils.h"

error_code compile(task_t* task, result_t* result) {
  if(strlen(task->src_path) > 0) {
    FILE* src_fp = fopen(task->src_path, "rb");
    if(src_fp != NULL) {
      fseek(src_fp, 0, SEEK_END);
      int len = (unsigned int) ftell(src_fp);
      fclose(src_fp);
      result->code_length = (unsigned int)len;
    } else {
      LOG_ERROR(log_fp, "Error fopen() src file %s", task->src_path);
      return BAD_SRC_FILE;
    }
  }

  if(strlen(task->compile_cmd) > 0) {
    char cmd_buf[256];
    strcpy(cmd_buf, "timeout -s KILL 3 nice -10 ");
    strcat(cmd_buf, task->compile_cmd);
    if (system(cmd_buf) != 0) {
      result->result = COMPILE_ERROR;
      LOG_ERROR(log_fp, "Error system() executing %s", cmd_buf);
      return COMPILE_FAILED;
    }
  }

  return SUCCESS;
}