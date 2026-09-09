#include "../../../acutest.h"
#include "../dynamic_list.c"

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

TEST_LIST = {
  { "int_bisects", test_bisect_int },
  { "float_bisect", test_bisect_float },
  { "int_duplicates", test_bisect_int_duplicates },
  { "int_edge_cases", test_bisect_int_edge_cases },
  { "struct_bisect", test_bisect_struct },
  { NULL, NULL }
};
