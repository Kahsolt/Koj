#include <string.h>
#include <stdio.h>

#include "comparor.h"
#include "utils.h"

error_code compare(task_t* task, result_t* result) {
  if (strlen(task->output_path) == 0) {
    result->result = ACCEPT;
    return SUCCESS;
  }

  FILE* output_fp = fopen(task->output_path, "rb");
  FILE* user_fp = fopen(task->ans_path, "rb");
  if (output_fp == NULL || user_fp == NULL) {
    if(output_fp == NULL) LOG_ERROR(log_fp, "Error open file %s", task->output_path);
    else fclose(output_fp);
    if(user_fp == NULL) LOG_ERROR(log_fp, "Error open file %s", task->ans_path);
    else fclose(user_fp);
    return FOPEN_FAILED;
  }

  int target, output, diff = 0;

  // Strict Match
  /* user_output must start with ans_output */
  while ((target = fgetc(output_fp)) != EOF)
    if(fgetc(user_fp) != target) { diff = 1; break; }
  /* extra user_output must be a blank char */
  while ((output = fgetc(user_fp)) != EOF)
    if (!(output == '\n' || output == '\r'
          || output == '\0'|| output == ' ')) { diff = 1; break; }

  if(diff == 0)
    result->result = ACCEPT;
  else {  // Non-strict Like (wind to file beginning first)
    diff = 0;
    fseek(output_fp, 0, SEEK_SET);
    fseek(user_fp, 0, SEEK_SET);

    /* if user_output contains all non-blank chars in ans_output */
    while (1) {
      do { target = fgetc(output_fp); }
      while (target != EOF && !(33 <= target && target <= 126));
      if (target == EOF) break;

      do { output = fgetc(user_fp); }
      while (output != EOF && !(33 <= output && output <= 126));
      if (output == EOF || output != target) { diff = 1; break; }
    }
    /* extra user_output must be a blank char */
    while ((output = fgetc(user_fp)) != EOF)
      if (!(output == '\n' || output == '\r'
            || output == '\0'|| output == ' ')) { diff = 1; break; }

    if(diff == 0)
      result->result = PRESENTATION_ERROR;
    else
      result->result = WRONG_ANSWER;
  }

  fclose(output_fp); fclose(user_fp);
  return SUCCESS;
}