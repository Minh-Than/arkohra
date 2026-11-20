#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "windows/rgui_input_data.h"
#include "libraries/ceval.h"
#include "raymath.h"
#include "resource_util.h"
#include "custom_types.h"

List parse_int_array(const char *str)
{
  List result;
  list_init(&result, sizeof(int));

  const char *p = strchr(str, '[');
  if (p == NULL) return result;
  p++;

  while (*p != '\0' && *p != ']')
  {
    int value;
    int chars_consumed = 0;

    int matched = sscanf(p, "%d%n", &value, &chars_consumed);
    if (matched != 1) break; // couldn't parse a number here — stop

    list_push(&result, &value);
    p += chars_consumed; // advance the pointer past what we just read

    while (*p == ',' || *p == ' ') p++; // skip the comma (and any stray spaces) before the next number
  }

  return result;
}

char *trim_whitespace(char *str)
{
  while (isspace((unsigned char)*str)) str++;

  if (*str == '\0') return str;

  char *end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end)) end--;
  end[1] = '\0';

  return str;
}

int split(char *str, char delimiter, token_fn fn, void *user)
{
  int count = 0;
  char *p = str;

  for (;;) {
    char *next = strchr(p, delimiter);
    char *tok = p;

    if (next) *next = '\0';
    trim_whitespace(tok);
    if (!fn(tok, user)) return -1;
    count++;
    if (!next) return count;
    p = next + 1;
  }
}

int text_to_int_validated(const char *text, int fallback, int min, int max)
{
  if (sizeof(text) > RGUI_INPUT_TEXT_CAP) return fallback;

  char expr[RGUI_INPUT_TEXT_CAP];
  text_copy_bounded(expr, sizeof(expr), text);

  int v = ceval_result(expr);
  if (isnan(v)) return fallback;

  return roundf(Clamp(v, min, max));
}

float text_to_float_validated(const char *text, float fallback, float min, float max)
{
  if (sizeof(text) > RGUI_INPUT_TEXT_CAP) return fallback;

  char expr[RGUI_INPUT_TEXT_CAP];
  text_copy_bounded(expr, sizeof(expr), text);

  float v = ceval_result(expr);
  if (isnan(v)) return fallback;

  return Clamp(v, min, max);
}

int count_decimals(const char *text)
{
  const char *dot = strchr(text, '.');
  if (dot == NULL) return 1;
  int n = 0;
  while (dot[1 + n] >= '0' && dot[1 + n] <= '9') n++;
  if (n == 0) n = 1;
  return n;
}

int double_compare_asc(const void *a, const void *b)
{
  const double *da = (const double *)a;
  const double *db = (const double *)b;
  if (*da < *db) return -1;
  if (*da > *db) return 1;
  return 0;
}

bool between_int_range_exclusive(int value, int low, int high)
{
  return low < value && value < high;
}

bool between_int_range_inclusive(int value, int low, int high)
{
  return low <= value && value <= high;
}
