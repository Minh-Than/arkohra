#include "../../../acutest.h"
#include "../custom_types.c"

int int_compare(const void *a, const void *b)
{
  const int *num_a = (const int *)a;
  const int *num_b = (const int *)b;
  return *num_a - *num_b;
}

int float_compare(const void *a, const void *b)
{
  const float *num_a = (const float *)a;
  const float *num_b = (const float *)b;
  if (*num_a < *num_b) return -1;
  if (*num_a > *num_b) return 1;
  return 0;
}

bool itv_result_contains(List *result, void *data)
{
  for (size_t k = 0; k < result->size; k++)
    if (*(void **)list_get(result, k) == data) return true;
  return false;
}

void test_itv_tree_insert_and_query(void)
{
  ItvTree tree; itv_tree_init(&tree, int_compare);

  int vals[] = {15, 20, 10, 30, 17, 19, 5, 20, 30, 40};
  int ids[]  = {0, 1, 2, 3, 4};
  Interval ivs[] = {
    {&vals[0], &vals[1]},  // [15,20]
    {&vals[2], &vals[3]},  // [10,30]
    {&vals[4], &vals[5]},  // [17,19]
    {&vals[6], &vals[7]},  // [5,20]
    {&vals[8], &vals[9]},  // [30,40]
  };
  for (int k = 0; k < 5; k++)
    TEST_CHECK(itv_tree_insert(&tree, &ids[k], ivs[k]));

  List result; list_init(&result, sizeof(void *));
  int ql, qh;

  // --- Only deep-left interval matches ---
  ql = 6; qh = 7;
  itv_tree_get_overlaps(&tree, &result, (Interval){&ql, &qh});
  TEST_CHECK(result.size == 1);
  TEST_MSG("[6,7]: expected 1 match, got %zu", result.size);
  TEST_CHECK(itv_result_contains(&result, &ids[3]));
  list_free(&result); list_init(&result, sizeof(void *));

  // --- Only [10,30] reaches past 20 ---
  ql = 21; qh = 23;
  itv_tree_get_overlaps(&tree, &result, (Interval){&ql, &qh});
  TEST_CHECK(result.size == 1);
  TEST_MSG("[21,23]: expected 1 match, got %zu", result.size);
  TEST_CHECK(itv_result_contains(&result, &ids[1]));
  list_free(&result); list_init(&result, sizeof(void *));

  // --- Point query, two touching matches ---
  ql = 30; qh = 30;
  itv_tree_get_overlaps(&tree, &result, (Interval){&ql, &qh});
  TEST_CHECK(result.size == 2);
  TEST_MSG("[30,30]: expected 2 matches, got %zu", result.size);
  TEST_CHECK(itv_result_contains(&result, &ids[1]));
  TEST_CHECK(itv_result_contains(&result, &ids[4]));
  list_free(&result); list_init(&result, sizeof(void *));

  // --- No matches ---
  ql = 0; qh = 3;
  itv_tree_get_overlaps(&tree, &result, (Interval){&ql, &qh});
  TEST_CHECK(result.size == 0);
  TEST_MSG("[0,3]: expected 0 matches, got %zu", result.size);
  list_free(&result); list_init(&result, sizeof(void *));

  // --- Wide query: everything but [30,40] ---
  ql = 16; qh = 19;
  itv_tree_get_overlaps(&tree, &result, (Interval){&ql, &qh});
  TEST_CHECK(result.size == 4);
  TEST_MSG("[16,19]: expected 4 matches, got %zu", result.size);
  for (int k = 0; k < 4; k++)
    TEST_CHECK(itv_result_contains(&result, &ids[k]));
  list_free(&result);

  itv_tree_free(&tree);
}

void test_itv_tree_max_invariant(void)
{
  ItvTree tree; itv_tree_init(&tree, int_compare);

  int vals[] = {15, 20, 10, 30, 17, 19, 5, 20, 30, 40};
  Interval ivs[] = {
    {&vals[0], &vals[1]}, {&vals[2], &vals[3]}, {&vals[4], &vals[5]},
    {&vals[6], &vals[7]}, {&vals[8], &vals[9]},
  };
  for (int k = 0; k < 5; k++)
    itv_tree_insert(&tree, &vals[k], ivs[k]);

  //        Expected shape (ties go right):
  //                 [15,20] max=40
  //                 /            |
  //        [10,30] max=30    [17,19] max=40
  //          /                           |
  //   [5,20] max=20                  [30,40] max=40
  if (!TEST_CHECK(tree.root != NULL)) { itv_tree_free(&tree); return; }
  if (!TEST_CHECK(tree.root->left != NULL && tree.root->right != NULL)) { itv_tree_free(&tree); return; }
  if (!TEST_CHECK(tree.root->left->left  != NULL && tree.root->left->right  == NULL)) { itv_tree_free(&tree); return; }
  if (!TEST_CHECK(tree.root->right->left == NULL && tree.root->right->right != NULL)) { itv_tree_free(&tree); return; }

  TEST_CHECK(*(const int *)tree.root->i.low == 15);
  TEST_CHECK(*(const int *)tree.root->left->i.low == 10);
  TEST_CHECK(*(const int *)tree.root->left->left->i.low == 5);
  TEST_CHECK(*(const int *)tree.root->right->i.low == 17);
  TEST_CHECK(*(const int *)tree.root->right->right->i.low == 30);

  TEST_CHECK(*(const int *)tree.root->max == 40);
  TEST_CHECK(*(const int *)tree.root->left->max == 30);
  TEST_CHECK(*(const int *)tree.root->left->left->max == 20);
  TEST_CHECK(*(const int *)tree.root->right->max == 40);
  TEST_CHECK(*(const int *)tree.root->right->right->max == 40);

  itv_tree_free(&tree);
}

void test_itv_tree_max_regression(void)
{
  // Regression: a max-shrink bug lowers root->max to 3 after inserting [1,3],
  // so a query overlapping [15,20] gets pruned at the root and finds nothing
  ItvTree tree; itv_tree_init(&tree, int_compare);

  int a[] = {15, 20};
  int b[] = {1, 3};
  int q[] = {16, 18};
  TEST_CHECK(itv_tree_insert(&tree, &a, (Interval){&a[0], &a[1]}));
  TEST_CHECK(itv_tree_insert(&tree, &b, (Interval){&b[0], &b[1]}));

  List result; list_init(&result, sizeof(void *));
  itv_tree_get_overlaps(&tree, &result, (Interval){&q[0], &q[1]});
  TEST_CHECK(result.size == 1);
  TEST_MSG("[16,18]: expected 1 match, got %zu", result.size);
  TEST_CHECK(itv_result_contains(&result, &a));
  list_free(&result);

  itv_tree_free(&tree);
}

void test_itv_tree_data_identity(void)
{
  ItvTree tree; itv_tree_init(&tree, int_compare);

  int vals[] = {15, 20, 10, 30, 17, 19, 5, 20, 30, 40};
  int ids[]  = {0, 1, 2, 3, 4};
  Interval ivs[] = {
    {&vals[0], &vals[1]}, {&vals[2], &vals[3]}, {&vals[4], &vals[5]},
    {&vals[6], &vals[7]}, {&vals[8], &vals[9]},
  };
  for (int k = 0; k < 5; k++)
    itv_tree_insert(&tree, &ids[k], ivs[k]);

  // Every payload must come back exactly once, unaltered
  List result; list_init(&result, sizeof(void *));
  int ql = 0, qh = 100;
  itv_tree_get_overlaps(&tree, &result, (Interval){&ql, &qh});
  TEST_CHECK(result.size == 5);
  TEST_MSG("wide query: expected 5 matches, got %zu", result.size);
  for (int k = 0; k < 5; k++)
  {
    TEST_CHECK(itv_result_contains(&result, &ids[k]));
    TEST_MSG("payload &ids[%d] missing from results", k);
  }
  list_free(&result);

  itv_tree_free(&tree);
}

void test_itv_tree_duplicates(void)
{
  ItvTree tree; itv_tree_init(&tree, int_compare);

  int lo = 10, hi = 20;
  int ids[] = {0, 1};
  TEST_CHECK(itv_tree_insert(&tree, &ids[0], (Interval){&lo, &hi}));
  TEST_CHECK(itv_tree_insert(&tree, &ids[1], (Interval){&lo, &hi}));

  List result; list_init(&result, sizeof(void *));
  itv_tree_get_overlaps(&tree, &result, (Interval){&lo, &hi});
  TEST_CHECK(result.size == 2);
  TEST_MSG("duplicates: expected 2 matches, got %zu", result.size);
  TEST_CHECK(itv_result_contains(&result, &ids[0]));
  TEST_CHECK(itv_result_contains(&result, &ids[1]));
  list_free(&result);

  itv_tree_free(&tree);
}

void test_itv_tree_point_query(void)
{
  ItvTree tree; itv_tree_init(&tree, int_compare);

  int vals[] = {5, 10, 10, 20, 20, 30};
  Interval ivs[] = {
    {&vals[0], &vals[1]},  // [5,10]
    {&vals[2], &vals[3]},  // [10,20]
    {&vals[4], &vals[5]},  // [20,30]
  };
  for (int k = 0; k < 3; k++)
    itv_tree_insert(&tree, &vals[k], ivs[k]);

  List result; list_init(&result, sizeof(void *));
  int t = 10;
  itv_tree_get_overlaps(&tree, &result, (Interval){&t, &t});
  TEST_CHECK(result.size == 2);  // [5,10] and [10,20] touch at 10; [20,30] does not
  TEST_MSG("point 10: expected 2 matches, got %zu", result.size);
  list_free(&result); list_init(&result, sizeof(void *));

  t = 20;
  itv_tree_get_overlaps(&tree, &result, (Interval){&t, &t});
  TEST_CHECK(result.size == 2);  // [10,20] and [20,30]
  TEST_MSG("point 20: expected 2 matches, got %zu", result.size);
  list_free(&result); list_init(&result, sizeof(void *));

  t = 31;
  itv_tree_get_overlaps(&tree, &result, (Interval){&t, &t});
  TEST_CHECK(result.size == 0);
  TEST_MSG("point 31: expected 0 matches, got %zu", result.size);
  list_free(&result);

  itv_tree_free(&tree);
}

void test_itv_tree_float_negative(void)
{
  ItvTree tree; itv_tree_init(&tree, float_compare);

  float a[] = {-10.5f, -3.2f};
  float b[] = {-5.0f, 5.0f};
  float c[] = {3.0f, 10.0f};
  itv_tree_insert(&tree, a, (Interval){&a[0], &a[1]});
  itv_tree_insert(&tree, b, (Interval){&b[0], &b[1]});
  itv_tree_insert(&tree, c, (Interval){&c[0], &c[1]});

  List result; list_init(&result, sizeof(void *));
  float q[] = {-4.0f, -2.0f};
  itv_tree_get_overlaps(&tree, &result, (Interval){&q[0], &q[1]});
  TEST_CHECK(result.size == 2);  // [-10.5,-3.2] and [-5,5]; [3,10] is out
  TEST_MSG("[-4,-2]: expected 2 matches, got %zu", result.size);
  TEST_CHECK(itv_result_contains(&result, a));
  TEST_CHECK(itv_result_contains(&result, b));
  list_free(&result);

  itv_tree_free(&tree);
}

void test_itv_tree_insert_failures(void)
{
  ItvTree tree; itv_tree_init(&tree, int_compare);

  int x = 5;
  TEST_CHECK(!itv_tree_insert(&tree, NULL, (Interval){&x, &x}));       // null payload
  TEST_CHECK(!itv_tree_insert(&tree, &x, (Interval){NULL, &x}));       // null low
  TEST_CHECK(!itv_tree_insert(&tree, &x, (Interval){&x, NULL}));       // null high

  // Tree still empty and usable after failed inserts
  List result; list_init(&result, sizeof(void *));
  itv_tree_get_overlaps(&tree, &result, (Interval){&x, &x});
  TEST_CHECK(result.size == 0);
  list_free(&result);

  TEST_CHECK(itv_tree_insert(&tree, &x, (Interval){&x, &x}));
  itv_tree_free(&tree);
}

void test_itv_tree_free(void)
{
  // Payloads point into stack arrays: itv_tree_free must free nodes only
  int vals[] = {15, 20, 10, 30};
  int ids[] = {0, 1};
  ItvTree tree; itv_tree_init(&tree, int_compare);
  itv_tree_insert(&tree, &ids[0], (Interval){&vals[0], &vals[1]});
  itv_tree_insert(&tree, &ids[1], (Interval){&vals[2], &vals[3]});
  itv_tree_free(&tree);

  TEST_CHECK(tree.root == NULL && tree.compare == NULL);

  // Reusable after free
  itv_tree_init(&tree, int_compare);
  TEST_CHECK(itv_tree_insert(&tree, &ids[0], (Interval){&vals[0], &vals[1]}));
  itv_tree_free(&tree);

  // Fresh tree and NULL are safe no-ops
  ItvTree fresh; itv_tree_init(&fresh, int_compare);
  itv_tree_free(&fresh);
  itv_tree_free(NULL);
}

struct TestStruct {
  int order;
  bool is_active;
};

int test_struct_compare_order_asc(const void *a, const void *b)
{
  const struct TestStruct *test_a = (const struct TestStruct *)a;
  const struct TestStruct *test_b = (const struct TestStruct *)b;
  return test_a->order - test_b->order;
}

void test_bisect_int(void)
{
  List int_list; list_init(&int_list, sizeof(int));

  // [1, 2, 3, 4, 5, 6]
  int vals[] = {1, 2, 3, 4, 5, 6};
  for (int i = 0; i < 6; i++)
  {
    list_push(&int_list, &vals[i]);
  }

  // --- Target exist in list ---
  int target = 2;
  int idx_left = bisect_left(&int_list, &target, int_compare);
  int idx_right = bisect_right(&int_list, &target, int_compare);
  TEST_CHECK(idx_left == 1);
  TEST_MSG("bisect_left: expected 1, got %d", idx_left);
  TEST_CHECK(idx_right == 2);
  TEST_MSG("bisect_right: expected 2, got %d", idx_right);

  // --- Target smaller than all ---
  int target_0 = 0;
  idx_left = bisect_left(&int_list, &target_0, int_compare);
  idx_right = bisect_right(&int_list, &target_0, int_compare);
  TEST_CHECK(idx_left == 0);
  TEST_MSG("bisect_left (smaller): expected 0, got %d", idx_left);
  TEST_CHECK(idx_right == 0);
  TEST_MSG("bisect_right (smaller): expected 0, got %d", idx_right);

  // --- Target larger than all ---
  int target_7 = 7;
  idx_left = bisect_left(&int_list, &target_7, int_compare);
  idx_right = bisect_right(&int_list, &target_7, int_compare);
  TEST_CHECK(idx_left == 6);
  TEST_MSG("bisect_left (larger): expected 6, got %d", idx_left);
  TEST_CHECK(idx_right == 6);
  TEST_MSG("bisect_right (larger): expected 6, got %d", idx_right);

  // --- Empty list ---
  List int_empty_list; list_init(&int_empty_list, sizeof(int));
  idx_left = bisect_left(&int_empty_list, &target, int_compare);
  idx_right = bisect_right(&int_empty_list, &target, int_compare);
  TEST_CHECK(idx_left == 0);
  TEST_MSG("bisect_left (empty): expected 0, got %d", idx_left);
  TEST_CHECK(idx_right == 0);
  TEST_MSG("bisect_right (empty): expected 0, got %d", idx_right);

  list_free(&int_list);
  list_free(&int_empty_list);
}

void test_bisect_float(void)
{
  List float_list; list_init(&float_list, sizeof(float));

  // [1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f]
  float vals[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
  for (int i = 0; i < 6; i++)
  {
    list_push(&float_list, &vals[i]);
  }

  // --- Target exist in list ---
  float target = 2.0f;
  int idx_left = bisect_left(&float_list, &target, float_compare);
  int idx_right = bisect_right(&float_list, &target, float_compare);
  TEST_CHECK(idx_left == 1);
  TEST_MSG("bisect_left: expected 1, got %d", idx_left);
  TEST_CHECK(idx_right == 2);
  TEST_MSG("bisect_right: expected 2, got %d", idx_right);

  // --- Target smaller than all ---
  float target_0 = 0.0f;
  idx_left = bisect_left(&float_list, &target_0, float_compare);
  idx_right = bisect_right(&float_list, &target_0, float_compare);
  TEST_CHECK(idx_left == 0);
  TEST_MSG("bisect_left (smaller): expected 0, got %d", idx_left);
  TEST_CHECK(idx_right == 0);
  TEST_MSG("bisect_right (smaller): expected 0, got %d", idx_right);

  // --- Target larger than all ---
  float target_7 = 7;
  idx_left = bisect_left(&float_list, &target_7, float_compare);
  idx_right = bisect_right(&float_list, &target_7, float_compare);
  TEST_CHECK(idx_left == 6);
  TEST_MSG("bisect_left (larger): expected 6, got %d", idx_left);
  TEST_CHECK(idx_right == 6);
  TEST_MSG("bisect_right (larger): expected 6, got %d", idx_right);

  // --- Empty list ---
  List float_empty_list; list_init(&float_empty_list, sizeof(float));
  idx_left = bisect_left(&float_empty_list, &target, float_compare);
  idx_right = bisect_right(&float_empty_list, &target, float_compare);
  TEST_CHECK(idx_left == 0);
  TEST_MSG("bisect_left (empty): expected 0, got %d", idx_left);
  TEST_CHECK(idx_right == 0);
  TEST_MSG("bisect_right (empty): expected 0, got %d", idx_right);

  list_free(&float_list);
  list_free(&float_empty_list);
}

void test_bisect_int_duplicates(void)
{
  List int_list; list_init(&int_list, sizeof(int));

  // [1, 2, 2, 2, 3, 4]
  int vals[] = {1, 2, 2, 2, 3, 4};
  for (int i = 0; i < 6; i++)
    list_push(&int_list, &vals[i]);

  // --- Left returns first equal, right returns after last equal ---
  int target = 2;
  int idx_left = bisect_left(&int_list, &target, int_compare);
  int idx_right = bisect_right(&int_list, &target, int_compare);
  TEST_CHECK(idx_left == 1);
  TEST_MSG("bisect_left (duplicates): expected 1 (first equal), got %d", idx_left);
  TEST_CHECK(idx_right == 4);
  TEST_MSG("bisect_right (duplicates): expected 4 (after last equal), got %d", idx_right);

  // --- All duplicates: [5, 5, 5, 5] ---
  List all_same; list_init(&all_same, sizeof(int));
  int fives[] = {5, 5, 5, 5};
  for (int i = 0; i < 4; i++)
    list_push(&all_same, &fives[i]);

  // target equals all elements
  target = 5;
  idx_left = bisect_left(&all_same, &target, int_compare);
  idx_right = bisect_right(&all_same, &target, int_compare);
  TEST_CHECK(idx_left == 0);
  TEST_MSG("all-dup bisect_left: expected 0, got %d", idx_left);
  TEST_CHECK(idx_right == 4);
  TEST_MSG("all-dup bisect_right: expected 4, got %d", idx_right);

  // target smaller than all duplicates
  target = 3;
  idx_left = bisect_left(&all_same, &target, int_compare);
  idx_right = bisect_right(&all_same, &target, int_compare);
  TEST_CHECK(idx_left == 0);
  TEST_MSG("all-dup bisect_left (smaller): expected 0, got %d", idx_left);
  TEST_CHECK(idx_right == 0);
  TEST_MSG("all-dup bisect_right (smaller): expected 0, got %d", idx_right);

  // target larger than all duplicates
  target = 7;
  idx_left = bisect_left(&all_same, &target, int_compare);
  idx_right = bisect_right(&all_same, &target, int_compare);
  TEST_CHECK(idx_left == 4);
  TEST_MSG("all-dup bisect_left (larger): expected 4, got %d", idx_left);
  TEST_CHECK(idx_right == 4);
  TEST_MSG("all-dup bisect_right (larger): expected 4, got %d", idx_right);

  list_free(&int_list);
  list_free(&all_same);
}

void test_bisect_int_edge_cases(void)
{
  // --- Between two elements (target not in list, within range) ---
  List int_list; list_init(&int_list, sizeof(int));
  int vals[] = {1, 3, 5, 7, 9};
  for (int i = 0; i < 5; i++)
    list_push(&int_list, &vals[i]);

  // target=4 is between 3 (idx 1) and 5 (idx 2)
  int target = 4;
  int idx_left = bisect_left(&int_list, &target, int_compare);
  int idx_right = bisect_right(&int_list, &target, int_compare);
  TEST_CHECK(idx_left == 2);
  TEST_MSG("between 3 and 5: bisect_left expected 2, got %d", idx_left);
  TEST_CHECK(idx_right == 2);
  TEST_MSG("between 3 and 5: bisect_right expected 2, got %d", idx_right);

  // --- Target equals first element ---
  target = 1;
  idx_left = bisect_left(&int_list, &target, int_compare);
  idx_right = bisect_right(&int_list, &target, int_compare);
  TEST_CHECK(idx_left == 0);
  TEST_MSG("first element: bisect_left expected 0, got %d", idx_left);
  TEST_CHECK(idx_right == 1);
  TEST_MSG("first element: bisect_right expected 1, got %d", idx_right);

  // --- Target equals last element ---
  target = 9;
  idx_left = bisect_left(&int_list, &target, int_compare);
  idx_right = bisect_right(&int_list, &target, int_compare);
  TEST_CHECK(idx_left == 4);
  TEST_MSG("last element: bisect_left expected 4, got %d", idx_left);
  TEST_CHECK(idx_right == 5);
  TEST_MSG("last element: bisect_right expected 5, got %d", idx_right);

  // --- Single element list ---
  List single; list_init(&single, sizeof(int));
  int one = 5;
  list_push(&single, &one);

  // target equals the single element
  target = 5;
  idx_left = bisect_left(&single, &target, int_compare);
  idx_right = bisect_right(&single, &target, int_compare);
  TEST_CHECK(idx_left == 0);
  TEST_MSG("single (equal): bisect_left expected 0, got %d", idx_left);
  TEST_CHECK(idx_right == 1);
  TEST_MSG("single (equal): bisect_right expected 1, got %d", idx_right);

  // target smaller than the single element
  target = 0;
  idx_left = bisect_left(&single, &target, int_compare);
  idx_right = bisect_right(&single, &target, int_compare);
  TEST_CHECK(idx_left == 0);
  TEST_MSG("single (smaller): bisect_left expected 0, got %d", idx_left);
  TEST_CHECK(idx_right == 0);
  TEST_MSG("single (smaller): bisect_right expected 0, got %d", idx_right);

  // target larger than the single element
  target = 9;
  idx_left = bisect_left(&single, &target, int_compare);
  idx_right = bisect_right(&single, &target, int_compare);
  TEST_CHECK(idx_left == 1);
  TEST_MSG("single (larger): bisect_left expected 1, got %d", idx_left);
  TEST_CHECK(idx_right == 1);
  TEST_MSG("single (larger): bisect_right expected 1, got %d", idx_right);

  list_free(&int_list);
  list_free(&single);
}

void test_bisect_struct(void)
{
  List struct_list; list_init(&struct_list, sizeof(struct TestStruct));

  // [{1,true}, {2,false}, {2,true}, {3,true}, {4,false}]
  struct TestStruct vals[] = {
    {1, true}, {2, false}, {2, true}, {3, true}, {4, false}
  };
  for (int i = 0; i < 5; i++)
    list_push(&struct_list, &vals[i]);

  // --- Target exists in list (with duplicate order) ---
  struct TestStruct target = { .order = 2, .is_active = false };
  int idx_left = bisect_left(&struct_list, &target, test_struct_compare_order_asc);
  int idx_right = bisect_right(&struct_list, &target, test_struct_compare_order_asc);
  TEST_CHECK(idx_left == 1);
  TEST_MSG("bisect_left (dup order): expected 1 (first equal), got %d", idx_left);
  TEST_CHECK(idx_right == 3);
  TEST_MSG("bisect_right (dup order): expected 3 (after last equal), got %d", idx_right);

  // --- Target smaller than all ---
  target.order = 0;
  idx_left = bisect_left(&struct_list, &target, test_struct_compare_order_asc);
  idx_right = bisect_right(&struct_list, &target, test_struct_compare_order_asc);
  TEST_CHECK(idx_left == 0);
  TEST_MSG("bisect_left (smaller): expected 0, got %d", idx_left);
  TEST_CHECK(idx_right == 0);
  TEST_MSG("bisect_right (smaller): expected 0, got %d", idx_right);

  // --- Target larger than all ---
  target.order = 99;
  idx_left = bisect_left(&struct_list, &target, test_struct_compare_order_asc);
  idx_right = bisect_right(&struct_list, &target, test_struct_compare_order_asc);
  TEST_CHECK(idx_left == 5);
  TEST_MSG("bisect_left (larger): expected 5, got %d", idx_left);
  TEST_CHECK(idx_right == 5);
  TEST_MSG("bisect_right (larger): expected 5, got %d", idx_right);

  // --- Empty list ---
  List empty; list_init(&empty, sizeof(struct TestStruct));
  target.order = 2;
  idx_left = bisect_left(&empty, &target, test_struct_compare_order_asc);
  idx_right = bisect_right(&empty, &target, test_struct_compare_order_asc);
  TEST_CHECK(idx_left == 0);
  TEST_MSG("bisect_left (empty): expected 0, got %d", idx_left);
  TEST_CHECK(idx_right == 0);
  TEST_MSG("bisect_right (empty): expected 0, got %d", idx_right);

  // --- Verify is_active doesn't affect ordering (comparator only uses order) ---
  struct TestStruct *first_2 = (struct TestStruct *)list_get(&struct_list, 1);
  struct TestStruct *second_2 = (struct TestStruct *)list_get(&struct_list, 2);
  TEST_CHECK(first_2->order == 2 && second_2->order == 2);
  TEST_MSG("both dup structs have order==2: first=%d second=%d", first_2->order, second_2->order);
  TEST_CHECK(first_2->is_active == false && second_2->is_active == true);
  TEST_MSG("is_active differs (comparator ignores it): first=%d second=%d", first_2->is_active, second_2->is_active);

  list_free(&struct_list);
  list_free(&empty);
}

void test_itv_tree_inverted_intervals(void)
{
  // Negative-BPM groups yield start_fp > end_fp; the tree must canonicalize
  ItvTree tree; itv_tree_init(&tree, int_compare);
  int a[] = {20, 10};   // inserted "inverted": low=20, high=10
  int b[] = {50, 40};
  itv_tree_insert(&tree, &a, (Interval){&a[0], &a[1]});
  itv_tree_insert(&tree, &b, (Interval){&b[0], &b[1]});
  List result; list_init(&result, sizeof(void *));

  int q = 15;   // point inside [10,20]
  itv_tree_get_overlaps(&tree, &result, (Interval){&q, &q});
  TEST_CHECK(result.size == 1);
  TEST_MSG("Expected 1; got %zu", result.size);
  TEST_CHECK(itv_result_contains(&result, &a));
  list_free(&result); list_init(&result, sizeof(void *));

  int ql = 12, qh = 18;   // partial overlap — the case containment-semantics miss
  itv_tree_get_overlaps(&tree, &result, (Interval){&ql, &qh});
  TEST_CHECK(result.size == 1);
  TEST_MSG("Expected 1; got %zu", result.size);
  TEST_CHECK(itv_result_contains(&result, &a));
  list_free(&result); list_init(&result, sizeof(void *));

  int wl = 5, wh = 100;
  itv_tree_get_overlaps(&tree, &result, (Interval){&wl, &wh});
  TEST_CHECK(result.size == 2);
  TEST_MSG("Expected 2; got %zu", result.size);
  list_free(&result);
  itv_tree_free(&tree);
}

TEST_LIST = {
  { "int_bisects", test_bisect_int },
  { "float_bisect", test_bisect_float },
  { "int_duplicates", test_bisect_int_duplicates },
  { "int_edge_cases", test_bisect_int_edge_cases },
  { "struct_bisect", test_bisect_struct },
  { "itv_insert_and_query", test_itv_tree_insert_and_query },
  { "itv_max_invariant", test_itv_tree_max_invariant },
  { "itv_max_regression", test_itv_tree_max_regression },
  { "itv_payload_identity", test_itv_tree_data_identity },
  { "itv_duplicates", test_itv_tree_duplicates },
  { "itv_point_query", test_itv_tree_point_query },
  { "itv_float_negative", test_itv_tree_float_negative },
  { "itv_insert_failures", test_itv_tree_insert_failures },
  { "itv_free", test_itv_tree_free },
  { "itv_inverted_intervals", test_itv_tree_inverted_intervals },
  { NULL, NULL }
};
