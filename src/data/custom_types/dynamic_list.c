#include <string.h>
#include <stdio.h>
#include "dynamic_list.h"

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
  void *slot = (char *)list->data + (list->size * list->elem_size); // data's address + item counts (in elem_size)
  memcpy(slot, item, list->elem_size);
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
