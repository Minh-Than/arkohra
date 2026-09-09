#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "./custom_types.h"

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

int double_compare_asc(const void *a, const void *b)
{
  const double *da = (const double *)a;
  const double *db = (const double *)b;
  if (*da < *db) return -1;
  if (*da > *db) return 1;
  return 0;
}
