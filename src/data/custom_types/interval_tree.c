#include "interval_tree.h"

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
