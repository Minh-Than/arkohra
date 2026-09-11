#ifndef CUSTOM_TYPES_H
#define CUSTOM_TYPES_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "dynamic_list.h"
#include "interval_tree.h" // IWYU pragma: export

// Parsing-related
List parse_int_array(const char *str);

// String-related
typedef int (*token_fn)(char *token, void *user);
char *trim_whitespace(char *str);
int split(char *str, char delimiter, token_fn fn, void *user);

// Comparators
int double_compare_asc(const void *a, const void *b);
bool between_int_range_exclusive(int value, int low, int high);
bool between_int_range_inclusive(int value, int low, int high);
#endif // CUSTOM_TYPES_H
