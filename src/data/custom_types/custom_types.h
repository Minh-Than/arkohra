#ifndef CUSTOM_TYPES_H
#define CUSTOM_TYPES_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Dynamically allocated list
typedef struct {
  void *data;
  size_t elem_size;
  size_t size;
  size_t capacity;
} List;

void list_init(List *list, size_t elem_size);
void list_push(List *list, const void *item);
void *list_get(List *list, size_t index);
bool list_update(List *list, size_t index, const void *item);
bool list_remove_last_swap(List *list, size_t index);
void list_sort_by(List *list, int (*compare)(const void *a, const void *b));
int cmp3(const void *a, const void *b);
int  bisect_right(List *list, const void *target, int (*compare)(const void *a, const void *b));
int bisect_left(List *list, const void *target, int (*compare)(const void *a, const void *b));
void list_free(List *list);
void list_print(const List *list, void (*print_elem)(const void *elem),
                const char *header);

// Parsing-related
List parse_int_array(const char *str);

// String-related
char *trim_whitespace(char *str);
#endif // CUSTOM_TYPES_H
