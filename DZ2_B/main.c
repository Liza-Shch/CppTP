#define _GNU_SOURCE
#define ERR_ "[error]"
#define MAX_STRING_LEN_ 2048
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *result_string; // строка результата
  char *rest_string; // остаток строки для дальнейшей обработки
  size_t result_string_length;
  size_t rest_string_length;
} string_part;

typedef string_part *(string_part_functional)(const char *string);
typedef char *(do_operation)(const char *left_string, const char *right_string);

const char plus = '+';
const char mul = '*';
const char open_bracket = '(';
const char close_bracket = ')';
const char *operations = "+*()";
const char quote = '"';
const char terminal_null = '\0';
const char *spaces = " \t";

string_part *element(const char *string);
string_part *multiplier(const char *string);
string_part *addend(const char *string);
string_part *expression(const char *string);

// чтение строки

char *input_string() {
  char *string = NULL;
  size_t size = 0;
  ssize_t bytes = getline(&string, &size, stdin);
  if (bytes < 1 || size > MAX_STRING_LEN_) {
    free(string);
    return NULL;
  }
  if (!string) {
    return NULL;
  }
  string[bytes] = terminal_null;
  return string;
}

// удаление разделителей в начале строки

const char *delete_space(const char *string) {
  char symbol = *string;
  while (strchr(spaces, symbol)) {
    string++;
    symbol = *string;
  }
  return string;
}

// создание структуры string_part

string_part *create_string_part(const char *result_string,
                                size_t result_string_length,
                                const char *rest_string,
                                size_t rest_string_length) {
  char wrong_data = !result_string || !rest_string;
  if (wrong_data) {
    return NULL;
  }
  string_part *result = (string_part *)malloc(sizeof(string_part));
  if (!result) {
    return NULL;
  }
  result->result_string =
      (char *)calloc(result_string_length + 1, sizeof(char));
  if (!result->result_string) {
    free(result);
    return NULL;
  }
  result->result_string_length = result_string_length;
  memmove(result->result_string, result_string,
          result_string_length * sizeof(char));
  result->result_string[result_string_length] = terminal_null;
  result->rest_string = (char *)calloc(rest_string_length + 1, sizeof(char));
  if (!result->rest_string) {
    free(result->result_string);
    free(result);
    return NULL;
  }
  result->rest_string_length = rest_string_length;
  memmove(result->rest_string, rest_string, rest_string_length * sizeof(char));
  result->rest_string[rest_string_length] = terminal_null;
  return result;
}

void free_string_part(string_part *object) {
  free(object->result_string);
  free(object->rest_string);
  free(object);
}

// объект <элемент> алфавита: "<строка>" | <число>

string_part *element(const char *string) {
  if (!string) {
    return NULL;
  }
  string = delete_space((char *)string);
  size_t result_string_iterator = 0;
  size_t result_string_size = sizeof(char) * strlen(string);
  char *result_string = (char *)calloc(result_string_size + 1, sizeof(char));
  if (!result_string) {
    return NULL;
  }
  if (*string == quote) {
    result_string[result_string_iterator] = quote;
    result_string_iterator++;
    string++;
    while (*string != terminal_null) {
      result_string[result_string_iterator] = *string;
      result_string_iterator++;
      if (*string == quote) {
        break;
      }
      string++;
    }
    if (*string == terminal_null) {
      free(result_string);
      return NULL;
    }
    string++;
  } else {
    if (!isdigit(*string)) {
      free(result_string);
      return NULL;
    }
    while (isdigit(*string)) {
      result_string[result_string_iterator] = *string;
      result_string_iterator++;
      string++;
      if (*string == terminal_null) {
        break;
      }
    }
  }
  string_part *result = create_string_part(result_string, strlen(result_string),
                                           string, strlen(string));
  free(result_string);
  return result;
}

// объект <множитель> = (<выражение>) | <элемент>

string_part *multiplier(const char *string) {
  if (!string) {
    return NULL;
  }
  string = delete_space(string);
  if (*string == open_bracket) { // множитель имеет вид: (<выражение>)
    string++;
    string_part *current = expression(string);
    if (current) {
      current->rest_string = (char *)delete_space(current->rest_string);
      if (current->rest_string_length != 0) {
        if (*current->rest_string == close_bracket) {
          string_part *result = create_string_part(
              current->result_string, current->result_string_length,
              current->rest_string + 1, current->rest_string_length - 1);
          free_string_part(current);
          return result;
        }
        free_string_part(current);
        return NULL;
      }
      return NULL;
    }
  }
  return element(string); // множитель имеет вид: <элемент>
}

// умножение строки на число

char *operation_mult(const char *string, int number) {
  if (!string || number < 0) {
    return NULL;
  }
  size_t string_length = strlen(string);
  if (string_length * number + 1 > MAX_STRING_LEN_) {
    return NULL;
  }
  char *result = (char *)calloc(MAX_STRING_LEN_, sizeof(char));
  if (!result) {
    return NULL;
  }
  if (number == 0) {
    result[0] = quote;
    result[1] = quote;
    result[2] = terminal_null;
    return result;
  }

  result[0] = quote;
  size_t count = 0;
  size_t iterator = 1;
  const size_t quotes_count = 2;
  string_length -= quotes_count;
  for (; count < number; count++) {
    for (iterator = 1; iterator < string_length + 1; iterator++) {
      result[count * string_length + iterator] = string[iterator];
    }
  }
  size_t last_symbol_position = (count - 1) * string_length + iterator;
  result[last_symbol_position] = quote;
  last_symbol_position++;
  result[last_symbol_position] = terminal_null;
  return result;
}

// преобразует множитель типа <число> в число
// проверка множителей на соответсвие: "<строка>" * <число> | <число> *
// "<строка>"
// вызов функции умножения

char *multiplication(const char *left_multiplier,
                     const char *right_multiplier) {
  if (!left_multiplier || !right_multiplier) {
    return NULL;
  }
  if (*left_multiplier == quote) {
    if (*right_multiplier != quote) {
      int number = atoi(right_multiplier);
      return operation_mult(left_multiplier, number);
    }
    return NULL;
  }
  if (*right_multiplier == quote) {
    int number = atoi(left_multiplier);
    return operation_mult(right_multiplier, number);
  }
  int number = atoi(left_multiplier) * atoi(right_multiplier);
  char *result = (char *)calloc(MAX_STRING_LEN_, sizeof(char));
  if (!result) {
    return NULL;
  }
  snprintf(result, MAX_STRING_LEN_, "%d", number);
  return result;
}

// сложение строк

char *addition(const char *left_multiplier, const char *right_multiplier) {
  if (!left_multiplier || !right_multiplier) {
    return NULL;
  }
  size_t left_multiplier_length = strlen(left_multiplier);
  size_t right_multiplier_length = strlen(right_multiplier);
  char wrong_data =
      *left_multiplier != quote || *right_multiplier != quote ||
      left_multiplier_length + right_multiplier_length + 1 > MAX_STRING_LEN_;
  if (wrong_data) {
    return NULL;
  }
  size_t result_length = left_multiplier_length + right_multiplier_length - 2;
  char *result = (char *)calloc(result_length + 1, sizeof(char));
  if (!result) {
    return NULL;
  }
  size_t left_string_copy_length = left_multiplier_length - 1;
  size_t iterator = 0;
  for (; iterator < left_string_copy_length; iterator++) {
    result[iterator] = left_multiplier[iterator];
  }
  right_multiplier++;
  snprintf(result + iterator, result_length, "%s", right_multiplier);
  result[result_length] = terminal_null;
  return result;
}

// получаем множитель или слагаемое

string_part *find_string_part(const char *string,
                              string_part_functional receive_string_part,
                              do_operation calculate, const char operation) {
  string_part *current = receive_string_part(string);
  if (!current) {
    return NULL;
  }
  char *left_result_string = (char *)calloc(MAX_STRING_LEN_, sizeof(char));
  if (!left_result_string) {
    free_string_part(current);
    return NULL;
  }
  size_t result_length = current->result_string_length;
  memmove(left_result_string, current->result_string,
          result_length * sizeof(char) + 1);
  left_result_string[result_length] = terminal_null;
  while (current && current->rest_string_length != 0) {
    string_part *mem_free_current = current;
    const char *buffer_rest_string = delete_space(current->rest_string);
    if (*buffer_rest_string != operation) {
      char current_operation = *buffer_rest_string;
      if (!strchr(operations, current_operation)) {
        free(left_result_string);
        free_string_part(mem_free_current);
        return NULL;
      }
      break;
    }
    buffer_rest_string++;
    current = receive_string_part(buffer_rest_string);
    free_string_part(mem_free_current);
    if (current) {
      char *mem_free_left_result_string = left_result_string;
      left_result_string =
          calculate(left_result_string, current->result_string);
      free(mem_free_left_result_string);
      if (!left_result_string) {
        free_string_part(current);
        return NULL;
      }
      mem_free_current = current;
      current =
          create_string_part(left_result_string, strlen(left_result_string),
                             current->rest_string, current->rest_string_length);
      free_string_part(mem_free_current);
    }
  }
  free(left_result_string);
  return current;
}

// получаем множитель

string_part *addend(const char *string) {
  if (!string) {
    return NULL;
  }
  return find_string_part(string, &multiplier, &multiplication, mul);
}

// получаем слагаемое
// любое выражение есть сумма слагаемых
// суммируем слагаемые, получаем выражение

string_part *expression(const char *string) {
  if (!string) {
    return NULL;
  }
  return find_string_part(string, &addend, &addition, plus);
}

int main() {
  char *string = input_string();
  if (!string) {
    printf("%s", ERR_);
    return 0;
  }
  string_part *result_string = expression(string);
  if (!result_string) {
    printf("%s", ERR_);
    free(string);
    return 0;
  }
  printf("%s", result_string->result_string);
  free(string);
  free_string_part(result_string);
  return 0;
}