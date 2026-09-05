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
int  cmp3(const void *a, const void *b);
int  bisect_right(List *list, const void *target, int (*compare)(const void *a, const void *b));
int  bisect_left(List *list, const void *target, int (*compare)(const void *a, const void *b));
void list_free(List *list);
void list_print(const List *list, void (*print_elem)(const void *elem),
                const char *header);

// Interval tree
typedef struct {
  const void *low, *high;
} Interval;

struct ItvNode {
  void *data;
  Interval i;
  const void *max;
  struct ItvNode *left, *right;
};

struct ItvNode *itv_node_init(void *data, Interval i);
struct ItvNode *itv_node_insert(struct ItvNode *node, void* data, Interval i, int (*compare)(const void *a, const void *b));
bool itv_node_check_overlap(Interval i1, Interval i2, int (*compare)(const void *a, const void *b));
void itv_node_get_overlaps(struct ItvNode *node, List *result, Interval i, int (*compare)(const void *a, const void *b));
void itv_node_free(struct ItvNode *node);

typedef struct {
  struct ItvNode *root;
  int (*compare)(const void *a, const void *b);
} ItvTree;

void itv_tree_init(ItvTree *tree, int (*compare)(const void *a, const void *b));
bool itv_tree_insert(ItvTree *tree, void *data, Interval i);
void itv_tree_get_overlaps(ItvTree *tree, List *result, Interval i);
void itv_tree_free(ItvTree *tree);

// Parsing-related
List parse_int_array(const char *str);

// String-related
char *trim_whitespace(char *str);

// Comparators
int double_compare_asc(const void *a, const void *b);
#endif // CUSTOM_TYPES_H
