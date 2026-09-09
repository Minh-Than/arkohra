#ifndef INTERVAL_TREE_H
#define INTERVAL_TREE_H

#include "dynamic_list.h"

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


#endif // INTERVAL_TREE_H
