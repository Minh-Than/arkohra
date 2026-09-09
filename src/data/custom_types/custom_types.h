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
char *trim_whitespace(char *str);

// Comparators
int double_compare_asc(const void *a, const void *b);
#endif // CUSTOM_TYPES_H
