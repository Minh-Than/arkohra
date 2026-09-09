#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "./custom_types.h"

void list_init(List *list, size_t elem_size)
{
  list->data        = NULL;
  list->elem_size   = elem_size;
  list->size        = list->capacity = 0;
}

void list_push(List *list, const void *item)
{
  if (list->size == list->capacity)
  {
    list->capacity  = list->capacity == 0 ? 4 : list->capacity * 2;
    list->data      = realloc(list->data, list->capacity * list->elem_size);
  }
  void *slot = (char *)list->data + (list->size * list->elem_size); // Grab a slot to push in new item
  memcpy(slot, item, list->elem_size); // Put that shi in the slot
  list->size++;
}

void *list_get(List *list, size_t index)
{
  if (index >= list->size) return NULL;
  return (char *)list->data + (index * list->elem_size);
}

void list_sort_by(List *list, int (*compare)(const void *a, const void *b))
{
  qsort(list->data, list->size, list->elem_size, compare);
}

int cmp3(const void *a, const void *b) {
    const double *pa = (const double *)a;
    const double *pb = (const double *)b;
    for (int i = 0; i < 3; i++) {
        if (pa[i] < pb[i]) return -1;
        if (pa[i] > pb[i]) return  1;
    }
    return 0;
}

int bisect_right(List *list, const void *target, int (*compare)(const void *a, const void *b))
{
  int low = 0;
  int high = (int)list->size;
  int mid;

  while (low < high)
  {
    mid = low + (high - low) / 2;
    void *mid_item = list_get(list, mid);
    if (compare(target, mid_item) < 0) high = mid;
    else low = mid + 1;
  }

  return low;
}

int bisect_left(List *list, const void *target, int (*compare)(const void *a, const void *b))
{
  int low = 0;
  int high = (int)list->size;
  int mid;

  while (low < high)
  {
    mid = low + (high - low) / 2;
    void *mid_item = list_get(list, mid);
    if (compare(target, mid_item) > 0) low = mid + 1;
    else high = mid;
  }

  return low;
}

void list_clear(List *list)
{
  if (!list) return;
  list->size = 0;
}

void list_free(List *list)
{
  if (!list) return;
  free(list->data);
  list->data = NULL;
  list->size = list->capacity = 0;
}

bool list_update(List *list, size_t index, const void *item)
{
  if (index >= list->size) return false;
  void *slot = (char *)list->data + (index * list->elem_size);
  memcpy(slot, item, list-> elem_size);
  return true;
}

bool list_remove_last_swap(List *list, size_t index)
{
  if (index >= list->size) return false;
  void *target = (char *)list->data + (index * list->elem_size);
  void *last   = (char *)list->data + ((list->size - 1) * list->elem_size);
  if (target != last) memcpy(target, last, list->elem_size);
  list->size--;
  return true;
}

void list_print(const List *list, void (*print_elem)(const void *elem), const char *header)
{
  printf("%s [", header);
  for (size_t i = 0; i < list->size; i++)
  {
    if (i == 0) printf("\n");
    const void *elem = (const char *)list->data + (i * list->elem_size);
    printf("  ");
    print_elem(elem);
    printf("\n");
  }
  printf("]\n");
}

struct ItvNode *itv_node_init(void *data, Interval i)
{
  if (!data) return NULL;
  if (!i.low || !i.high) return NULL;
  struct ItvNode *node = malloc(sizeof(struct ItvNode));
  if (!node) return NULL;

  node->data = data;
  node->i = i;
  node->max = i.high;
  node->left = node->right = NULL;

  return node;
}

struct ItvNode *itv_node_insert(struct ItvNode *node, void* data, Interval i, int (*compare)(const void *a, const void *b))
{
  if (!data) return node;
  if (!i.low || !i.high) return node;

  if (compare(i.low, i.high) > 0)
  {
    const void *tmp = i.low;
    i.low = i.high;
    i.high = tmp;
  }
  if (!node) return itv_node_init(data, i);

  if (compare(i.high, node->max) > 0) node->max = i.high;
  if (compare(i.low, node->i.low) < 0)
    node->left = itv_node_insert(node->left, data, i, compare);
  else
    node->right = itv_node_insert(node->right, data, i, compare);
  return node;
}

bool itv_node_check_overlap(Interval i1, Interval i2, int (*compare)(const void *a, const void *b))
{
  return compare(i1.low, i2.high) <= 0 && compare(i2.low, i1.high) <= 0;
}

void itv_node_get_overlaps(struct ItvNode *node, List *result, Interval i, int (*compare)(const void *a, const void *b))
{
  if(!node) return;

  // Return if i.low > node.max
  if(compare(node->max, i.low) < 0) return;

  // Process node's left subtree if that subtree contains i.low
  if(node->left && compare(node->left->max, i.low) >= 0)
    itv_node_get_overlaps(node->left, result, i, compare);

  if (itv_node_check_overlap(node->i, i, compare))
    list_push(result, &node->data);

  // Process node's right subtree if that subtree contains i.high
  if(node->right && compare(node->i.low, i.high) <= 0)
    itv_node_get_overlaps(node->right, result, i, compare);
}

void itv_node_free(struct ItvNode *node)
{
  if (!node) return;
  itv_node_free(node->left);
  itv_node_free(node->right);
  free(node);
}

void itv_tree_init(ItvTree *tree, int (*compare)(const void *a, const void *b))
{
  tree->root = NULL;
  tree->compare = compare;
}

bool itv_tree_insert(ItvTree *tree, void *data, Interval i)
{
  struct ItvNode *res = itv_node_insert(tree->root, data, i, tree->compare);
  if (!res) return false;
  tree->root = res;
  return true;
}

void itv_tree_get_overlaps(ItvTree *tree, List *result, Interval i)
{
  itv_node_get_overlaps(tree->root, result, i, tree->compare);
}

void itv_tree_free(ItvTree *tree)
{
  if (!tree) return;
  itv_node_free(tree->root);
  tree->root = NULL;
  tree->compare = NULL;
}

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
