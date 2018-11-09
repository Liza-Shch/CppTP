#define _GNU_SOURCE
#define ERR_ "[error]"
#define MAX_STRING_LEN_ 1024
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **input(int *strings_count) {
  char **strings = NULL;
  char *buffer = (char *)malloc(sizeof(char) * MAX_STRING_LEN_);
  while (fgets(buffer, MAX_STRING_LEN_, stdin)) {
    if (*buffer == '.') {
      break;
    }
    (*strings_count)++;
    char **strings_memory_error = strings;
    strings =
        (char **)realloc(strings_memory_error, sizeof(char *) * *strings_count);
    if (!strings) {
      printf("%s", ERR_);
      free(strings_memory_error);
      free(buffer);
      return NULL;
    }
    size_t buffer_len = strlen(buffer);
    strings[*strings_count - 1] = (char *)malloc(sizeof(char) * buffer_len + 1);
    if (!strings[*strings_count - 1]) {
      printf("%s", ERR_);
      free(buffer);
      return NULL;
    }
    memmove(strings[*strings_count - 1], buffer, sizeof(char) * buffer_len + 1);
  }

  free(buffer);
  return strings;
}

int find_right_braces_seq(int strings_count, const char **strings,
                          char **result_strings) {
  if (strings_count < 0 || !strings || !result_strings) {
    printf("%s", ERR_);
    return 0;
  }

  int result_strings_count = 0;
  for (int string_num = 0; string_num < strings_count; string_num++) {
    int count_open = 0;
    int count_close = 0;
    size_t string_len = strlen(strings[string_num]);
    for (size_t symbol_num = 0; symbol_num < string_len; symbol_num++) {
      if (strings[string_num][symbol_num] == '(') {
        count_open++;
      } else {
        if (strings[string_num][symbol_num] == ')') {
          count_close++;
        }
      }
    }

    if ((count_open == count_close)) {
      result_strings[result_strings_count] =
          (char *)malloc(sizeof(char) * string_len + 1);
      if (!result_strings[result_strings_count]) {
        printf("%s", ERR_);
        return 0;
      }
      strncpy(result_strings[result_strings_count], strings[string_num],
              sizeof(char) * string_len + 1);
      result_strings[result_strings_count][sizeof(char) * string_len] = '\0';
      result_strings_count++;
    }
  }
  return result_strings_count;
}

void print(int strings_count, const char **strings) {
  if (strings_count < 0 || !strings) {
    printf("%s", ERR_);
    return;
  }

  for (int string_num = 0; string_num < strings_count; string_num++) {
    printf("%s", strings[string_num]);
  }
}

void free_strings(int strings_count, char **strings) {
  if (strings_count < 0 || !strings) {
    printf("%s", ERR_);
    return;
  }
  for (int strings_num = 0; strings_num < strings_count; strings_num++) {
    free(strings[strings_num]);
  }
}

int main() {
  int strings_count = 0;
  char **strings = input(&strings_count);
  if (!strings) {
    printf("%s", ERR_);
    return 0;
  }
  char **result_strings = (char **)malloc(sizeof(char *) * strings_count);
  if (!result_strings) {
    printf("%s", ERR_);
    return 0;
  }
  int result_strings_count = find_right_braces_seq(
      strings_count, (const char **)strings, result_strings);
  if (result_strings_count == 0) {
    free_strings(strings_count, strings);
    free(strings);
    free(result_strings);
    return 0;
  }
  char **return_strings =
      (char **)realloc(result_strings, sizeof(char *) * result_strings_count);
  if (!return_strings) {
    printf("%s", ERR_);
    free(result_strings);
    return 0;
  }
  print(result_strings_count, (const char **)return_strings);

  free_strings(strings_count, strings);
  free_strings(result_strings_count, return_strings);
  free(strings);
  free(return_strings);
  return 0;
}