#include "datastructures.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─────────────────────────────────────────────
   Helpers
───────────────────────────────────────────── */

static int tests_run    = 0;
static int tests_passed = 0;

#define TEST(name) \
    do { \
        printf("  %-55s", name); \
        tests_run++; \
    } while(0)

#define PASS() \
    do { \
        printf("PASS\n"); \
        tests_passed++; \
    } while(0)

#define FAIL(msg) \
    do { \
        printf("FAIL  (%s:%d) %s\n", __FILE__, __LINE__, msg); \
    } while(0)

#define CHECK(cond, msg) \
    do { \
        if (!(cond)) { FAIL(msg); return; } \
    } while(0)

/* ─────────────────────────────────────────────
   Custom structs
───────────────────────────────────────────── */

typedef struct { int x; int y; } Point;
typedef struct { char name[32]; int age; float score; } Person;

static Point make_point(int x, int y) { Point p = {x, y}; return p; }
static Person make_person(const char *name, int age, float score) {
    Person p;
    strncpy(p.name, name, sizeof(p.name) - 1);
    p.name[sizeof(p.name) - 1] = '\0';
    p.age = age; p.score = score;
    return p;
}

/* ═══════════════════════════════════════════
   TEST GROUPS
═══════════════════════════════════════════ */

/* ── 1. Init / Free ─────────────────────── */

static void test_init_basic(void) {
    TEST("init: basic initialisation");
    linked_list_s *l = NULL;
    CHECK(llist_init(&l, sizeof(int)) == DS_OK, "init returned error");
    CHECK(l != NULL,            "list is NULL");
    CHECK(l->size == 0,         "size != 0");
    CHECK(l->head == NULL,      "head != NULL");
    llist_free(l);
    PASS();
}

static void test_init_custom_structs(void) {
    TEST("init: Point and Person elem sizes");
    linked_list_s *lp = NULL, *lper = NULL;
    CHECK(llist_init(&lp,   sizeof(Point))  == DS_OK, "Point init failed");
    CHECK(llist_init(&lper, sizeof(Person)) == DS_OK, "Person init failed");
    llist_free(lp);
    llist_free(lper);
    PASS();
}

static void test_free_empty(void) {
    TEST("free: empty list is safe");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    CHECK(llist_free(l) == DS_OK, "free empty returned error");
    PASS();
}

/* ── 2. Push Front ──────────────────────── */

static void test_push_front_single(void) {
    TEST("push_front: single int, head set correctly");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int val = 42;
    CHECK(llist_push_front(l, &val) == DS_OK, "push_front failed");
    CHECK(l->size == 1,   "size != 1");
    CHECK(l->head != NULL, "head is NULL");
    int out = 0;
    llist_get(l, 0, &out);
    CHECK(out == 42, "value wrong");
    llist_free(l);
    PASS();
}

static void test_push_front_multiple(void) {
    TEST("push_front: multiple ints, order reversed");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    for (int i = 0; i < 5; i++) llist_push_front(l, &i);
    /* expected: [4,3,2,1,0] */
    for (int i = 0; i < 5; i++) {
        int out = -1;
        llist_get(l, (size_t)i, &out);
        CHECK(out == 4 - i, "order wrong after push_front");
    }
    llist_free(l);
    PASS();
}

static void test_push_front_point(void) {
    TEST("push_front: Point struct fields preserved");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(Point));
    Point p = make_point(7, 13);
    llist_push_front(l, &p);
    Point out = {0, 0};
    llist_get(l, 0, &out);
    CHECK(out.x == 7 && out.y == 13, "Point fields wrong");
    llist_free(l);
    PASS();
}

/* ── 3. Push Back ───────────────────────── */

static void test_push_back_single(void) {
    TEST("push_back: single int into empty list");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int val = 99;
    CHECK(llist_push_back(l, &val) == DS_OK, "push_back failed");
    CHECK(l->size == 1, "size != 1");
    int out = 0;
    llist_get(l, 0, &out);
    CHECK(out == 99, "value wrong");
    llist_free(l);
    PASS();
}

static void test_push_back_multiple(void) {
    TEST("push_back: multiple ints, order preserved");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    for (int i = 0; i < 5; i++) llist_push_back(l, &i);
    /* expected: [0,1,2,3,4] */
    for (int i = 0; i < 5; i++) {
        int out = -1;
        llist_get(l, (size_t)i, &out);
        CHECK(out == i, "order wrong after push_back");
    }
    llist_free(l);
    PASS();
}

static void test_push_back_person(void) {
    TEST("push_back: Person struct fields preserved");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(Person));
    Person p = make_person("Alice", 30, 9.5f);
    llist_push_back(l, &p);
    Person out; memset(&out, 0, sizeof(out));
    llist_get(l, 0, &out);
    CHECK(strcmp(out.name, "Alice") == 0, "name wrong");
    CHECK(out.age == 30,                  "age wrong");
    llist_free(l);
    PASS();
}

/* ── 4. Pop Front ───────────────────────── */

static void test_pop_front_basic(void) {
    TEST("pop_front: returns front, shrinks size");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int a = 1, b = 2, c = 3;
    llist_push_back(l, &a); llist_push_back(l, &b); llist_push_back(l, &c);
    int out = 0;
    CHECK(llist_pop_front(l, &out) == DS_OK, "pop_front failed");
    CHECK(out == 1,          "wrong value");
    CHECK(l->size == 2,      "size wrong");
    llist_free(l);
    PASS();
}

static void test_pop_front_all(void) {
    TEST("pop_front: drain entire list");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    for (int i = 0; i < 5; i++) llist_push_back(l, &i);
    for (int i = 0; i < 5; i++) {
        int out = -1;
        CHECK(llist_pop_front(l, &out) == DS_OK, "pop_front failed");
        CHECK(out == i, "wrong pop order");
    }
    CHECK(l->size == 0,    "size should be 0");
    CHECK(l->head == NULL, "head should be NULL");
    llist_free(l);
    PASS();
}

static void test_pop_front_empty(void) {
    TEST("pop_front: empty list returns DS_ERR_EMPTY");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int out = 0;
    CHECK(llist_pop_front(l, &out) == DS_ERR_EMPTY, "expected EMPTY error");
    llist_free(l);
    PASS();
}

static void test_pop_front_null_out(void) {
    TEST("pop_front: NULL element_out doesn't crash");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int val = 5; llist_push_back(l, &val);
    CHECK(llist_pop_front(l, NULL) == DS_OK, "pop_front with NULL failed");
    CHECK(l->size == 0, "size wrong");
    llist_free(l);
    PASS();
}

/* ── 5. Pop Back ────────────────────────── */

static void test_pop_back_basic(void) {
    TEST("pop_back: returns back, shrinks size");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int a = 1, b = 2, c = 3;
    llist_push_back(l, &a); llist_push_back(l, &b); llist_push_back(l, &c);
    int out = 0;
    CHECK(llist_pop_back(l, &out) == DS_OK, "pop_back failed");
    CHECK(out == 3,     "wrong value");
    CHECK(l->size == 2, "size wrong");
    llist_free(l);
    PASS();
}

static void test_pop_back_single(void) {
    TEST("pop_back: single element, head becomes NULL");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int val = 77; llist_push_back(l, &val);
    int out = 0;
    CHECK(llist_pop_back(l, &out) == DS_OK, "pop_back failed");
    CHECK(out == 77,       "wrong value");
    CHECK(l->size == 0,    "size wrong");
    CHECK(l->head == NULL, "head should be NULL");
    llist_free(l);
    PASS();
}

static void test_pop_back_empty(void) {
    TEST("pop_back: empty list returns DS_ERR_EMPTY");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int out = 0;
    CHECK(llist_pop_back(l, &out) == DS_ERR_EMPTY, "expected EMPTY error");
    llist_free(l);
    PASS();
}

static void test_pop_back_null_out(void) {
    TEST("pop_back: NULL element_out doesn't crash");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int val = 5; llist_push_back(l, &val);
    CHECK(llist_pop_back(l, NULL) == DS_OK, "pop_back with NULL failed");
    CHECK(l->size == 0, "size wrong");
    llist_free(l);
    PASS();
}

static void test_pop_back_point(void) {
    TEST("pop_back: Point struct value correct");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(Point));
    Point a = make_point(1, 1), b = make_point(9, 9);
    llist_push_back(l, &a); llist_push_back(l, &b);
    Point out = {0, 0};
    llist_pop_back(l, &out);
    CHECK(out.x == 9 && out.y == 9, "Point wrong");
    llist_free(l);
    PASS();
}

/* ── 6. Get ─────────────────────────────── */

static void test_get_correct_values(void) {
    TEST("get: values match what was pushed");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    for (int i = 0; i < 8; i++) llist_push_back(l, &i);
    for (int i = 0; i < 8; i++) {
        int out = -1;
        CHECK(llist_get(l, (size_t)i, &out) == DS_OK, "get failed");
        CHECK(out == i, "value mismatch");
    }
    llist_free(l);
    PASS();
}

static void test_get_oob(void) {
    TEST("get: out-of-bounds returns DS_ERR_OUT_OF_BOUNDS");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int val = 1; llist_push_back(l, &val);
    int out = 0;
    CHECK(llist_get(l, 5, &out) == DS_ERR_OUT_OF_BOUNDS, "expected OOB");
    llist_free(l);
    PASS();
}

static void test_get_empty(void) {
    TEST("get: empty list returns DS_ERR_OUT_OF_BOUNDS");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int out = 0;
    CHECK(llist_get(l, 0, &out) == DS_ERR_OUT_OF_BOUNDS, "expected OOB");
    llist_free(l);
    PASS();
}

/* ── 7. Set ─────────────────────────────── */

static void test_set_basic(void) {
    TEST("set: overwrites existing value");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int val = 10; llist_push_back(l, &val);
    val = 99;
    CHECK(llist_set(l, &val, 0) == DS_OK, "set failed");
    int out = 0; llist_get(l, 0, &out);
    CHECK(out == 99, "value not updated");
    llist_free(l);
    PASS();
}

static void test_set_point(void) {
    TEST("set: overwrites Point struct");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(Point));
    Point p = make_point(1, 2); llist_push_back(l, &p);
    Point newp = make_point(7, 8);
    CHECK(llist_set(l, &newp, 0) == DS_OK, "set failed");
    Point out = {0, 0}; llist_get(l, 0, &out);
    CHECK(out.x == 7 && out.y == 8, "Point not updated");
    llist_free(l);
    PASS();
}

static void test_set_oob(void) {
    TEST("set: out-of-bounds returns DS_ERR_OUT_OF_BOUNDS");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int val = 5; llist_push_back(l, &val);
    CHECK(llist_set(l, &val, 10) == DS_ERR_OUT_OF_BOUNDS, "expected OOB");
    llist_free(l);
    PASS();
}

/* ── 8. Insert ──────────────────────────── */

static void test_insert_front(void) {
    TEST("insert: index 0 shifts all elements right");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    for (int i = 1; i <= 4; i++) llist_push_back(l, &i); /* [1,2,3,4] */
    int val = 99;
    CHECK(llist_insert(l, &val, 0) == DS_OK, "insert failed");
    int out = 0; llist_get(l, 0, &out);
    CHECK(out == 99,     "front value wrong");
    CHECK(l->size == 5,  "size wrong");
    llist_free(l);
    PASS();
}

static void test_insert_middle(void) {
    TEST("insert: middle index, sequence correct");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int vals[] = {1, 2, 4, 5};
    for (int i = 0; i < 4; i++) llist_push_back(l, &vals[i]);
    int val = 3;
    CHECK(llist_insert(l, &val, 2) == DS_OK, "insert failed");
    for (int i = 0; i < 5; i++) {
        int out = 0; llist_get(l, (size_t)i, &out);
        CHECK(out == i + 1, "sequence broken");
    }
    llist_free(l);
    PASS();
}

static void test_insert_back(void) {
    TEST("insert: index == size behaves like push_back");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int a = 1, b = 2; llist_push_back(l, &a);
    CHECK(llist_insert(l, &b, 1) == DS_OK, "insert at end failed");
    int out = 0; llist_get(l, 1, &out);
    CHECK(out == 2, "value wrong");
    llist_free(l);
    PASS();
}

static void test_insert_empty(void) {
    TEST("insert: index 0 into empty list");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int val = 5;
    CHECK(llist_insert(l, &val, 0) == DS_OK, "insert into empty failed");
    CHECK(l->size == 1, "size wrong");
    int out = 0; llist_get(l, 0, &out);
    CHECK(out == 5, "value wrong");
    llist_free(l);
    PASS();
}

static void test_insert_oob(void) {
    TEST("insert: beyond size returns DS_ERR_OUT_OF_BOUNDS");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int val = 1; llist_push_back(l, &val);
    CHECK(llist_insert(l, &val, 5) == DS_ERR_OUT_OF_BOUNDS, "expected OOB");
    llist_free(l);
    PASS();
}

/* ── 9. Remove ──────────────────────────── */

static void test_remove_front(void) {
    TEST("remove: index 0 shifts all elements left");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    for (int i = 0; i < 5; i++) llist_push_back(l, &i); /* [0,1,2,3,4] */
    CHECK(llist_remove(l, 0) == DS_OK, "remove failed");
    int out = 0; llist_get(l, 0, &out);
    CHECK(out == 1,     "front wrong after remove");
    CHECK(l->size == 4, "size wrong");
    llist_free(l);
    PASS();
}

static void test_remove_middle(void) {
    TEST("remove: middle index compacts correctly");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int vals[] = {1, 2, 99, 3, 4};
    for (int i = 0; i < 5; i++) llist_push_back(l, &vals[i]);
    CHECK(llist_remove(l, 2) == DS_OK, "remove failed");
    for (int i = 0; i < 4; i++) {
        int out = 0; llist_get(l, (size_t)i, &out);
        CHECK(out == i + 1, "sequence broken after remove");
    }
    llist_free(l);
    PASS();
}

static void test_remove_last(void) {
    TEST("remove: last element decrements size");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int a = 1, b = 2; llist_push_back(l, &a); llist_push_back(l, &b);
    CHECK(llist_remove(l, 1) == DS_OK, "remove last failed");
    CHECK(l->size == 1, "size wrong");
    llist_free(l);
    PASS();
}

static void test_remove_oob(void) {
    TEST("remove: out-of-bounds returns DS_ERR_OUT_OF_BOUNDS");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int val = 1; llist_push_back(l, &val);
    CHECK(llist_remove(l, 5) == DS_ERR_OUT_OF_BOUNDS, "expected OOB");
    llist_free(l);
    PASS();
}

static void test_remove_empty(void) {
    TEST("remove: empty list returns DS_ERR_OUT_OF_BOUNDS");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    CHECK(llist_remove(l, 0) == DS_ERR_OUT_OF_BOUNDS, "expected OOB");
    llist_free(l);
    PASS();
}

/* ── 10. Edge / Stress ──────────────────── */

static void test_push_front_back_interleaved(void) {
    TEST("edge: interleaved push_front and push_back");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int a = 2, b = 3, c = 1, d = 4;
    llist_push_back(l, &a);   /* [2] */
    llist_push_back(l, &b);   /* [2,3] */
    llist_push_front(l, &c);  /* [1,2,3] */
    llist_push_back(l, &d);   /* [1,2,3,4] */
    for (int i = 0; i < 4; i++) {
        int out = 0; llist_get(l, (size_t)i, &out);
        CHECK(out == i + 1, "interleaved order wrong");
    }
    llist_free(l);
    PASS();
}

static void test_large_volume(void) {
    TEST("edge: 10 000 ints pushed then verified");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    for (int i = 0; i < 10000; i++) llist_push_back(l, &i);
    CHECK(l->size == 10000, "size wrong");
    for (int i = 0; i < 10000; i++) {
        int out = -1; llist_get(l, (size_t)i, &out);
        CHECK(out == i, "value mismatch at large volume");
    }
    llist_free(l);
    PASS();
}

static void test_single_element_all_ops(void) {
    TEST("edge: single-element list survives get/set/pop");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    int val = 42; llist_push_back(l, &val);
    int out = 0;
    CHECK(llist_get(l, 0, &out) == DS_OK && out == 42, "get failed");
    val = 77;
    CHECK(llist_set(l, &val, 0) == DS_OK, "set failed");
    llist_get(l, 0, &out); CHECK(out == 77, "set value wrong");
    CHECK(llist_pop_front(l, &out) == DS_OK && out == 77, "pop failed");
    CHECK(l->size == 0 && l->head == NULL, "list not empty after pop");
    llist_free(l);
    PASS();
}

static void test_repeated_insert_remove_front(void) {
    TEST("edge: repeated insert/remove at front preserves data");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    for (int i = 0; i < 8; i++) llist_push_back(l, &i);
    for (int i = 0; i < 50; i++) {
        int val = 99;
        llist_insert(l, &val, 0);
        llist_remove(l, 0);
    }
    CHECK(l->size == 8, "size corrupted");
    for (int i = 0; i < 8; i++) {
        int out = -1; llist_get(l, (size_t)i, &out);
        CHECK(out == i, "data corrupted after repeated insert/remove");
    }
    llist_free(l);
    PASS();
}

static void test_person_data_integrity(void) {
    TEST("edge: Person data intact across many push_backs");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(Person));
    for (int i = 0; i < 100; i++) {
        char name[32]; snprintf(name, sizeof(name), "Person%d", i);
        Person p = make_person(name, i, (float)i * 0.5f);
        CHECK(llist_push_back(l, &p) == DS_OK, "push_back failed");
    }
    for (int i = 0; i < 100; i++) {
        Person out; memset(&out, 0, sizeof(out));
        llist_get(l, (size_t)i, &out);
        CHECK(out.age == i, "age corrupted");
    }
    llist_free(l);
    PASS();
}

static void test_pop_front_back_alternating(void) {
    TEST("edge: alternating pop_front and pop_back");
    linked_list_s *l = NULL;
    llist_init(&l, sizeof(int));
    for (int i = 1; i <= 6; i++) llist_push_back(l, &i); /* [1,2,3,4,5,6] */
    int out;
    llist_pop_front(l, &out); CHECK(out == 1, "pop_front wrong");
    llist_pop_back(l,  &out); CHECK(out == 6, "pop_back wrong");
    llist_pop_front(l, &out); CHECK(out == 2, "pop_front wrong");
    llist_pop_back(l,  &out); CHECK(out == 5, "pop_back wrong");
    CHECK(l->size == 2, "size wrong");
    llist_free(l);
    PASS();
}

/* ── Foreach ────────────────────────────── */

static void ll_sum_callback(const void *element, size_t index, void *user_data) {
    (void)index;
    int *sum = user_data;
    *sum += *(const int *)element;
}

static int ll_call_count = 0;
static void ll_count_callback(const void *element, size_t index, void *user_data) {
    (void)element; (void)index; (void)user_data;
    ll_call_count++;
}

static int ll_order_expected = 0;
static int ll_order_correct = 1;
static void ll_order_callback(const void *element, size_t index, void *user_data) {
    (void)user_data;
    if (*(const int *)element != ll_order_expected || (int)index != ll_order_expected)
        ll_order_correct = 0;
    ll_order_expected++;
}

static void test_llist_foreach_basic(void) {
    TEST("foreach: visits all elements, sum matches");
    linked_list_s *list = NULL;
    llist_init(&list, sizeof(int));
    int expected = 0;
    for (int i = 1; i <= 10; i++) {
        llist_push_back(list, &i);
        expected += i;
    }
    int sum = 0;
    CHECK(llist_foreach(list, ll_sum_callback, &sum) == DS_OK, "foreach failed");
    CHECK(sum == expected, "sum mismatch");
    llist_free(list);
    PASS();
}

static void test_llist_foreach_empty(void) {
    TEST("foreach: empty list, callback never called");
    linked_list_s *list = NULL;
    llist_init(&list, sizeof(int));
    ll_call_count = 0;
    CHECK(llist_foreach(list, ll_count_callback, NULL) == DS_OK, "foreach failed");
    CHECK(ll_call_count == 0, "callback called on empty list");
    llist_free(list);
    PASS();
}

static void test_llist_foreach_order(void) {
    TEST("foreach: visits elements head to tail");
    linked_list_s *list = NULL;
    llist_init(&list, sizeof(int));
    for (int i = 0; i < 5; i++)
        llist_push_back(list, &i);
    ll_order_expected = 0;
    ll_order_correct = 1;
    llist_foreach(list, ll_order_callback, NULL);
    CHECK(ll_order_correct == 1, "elements visited out of order");
    llist_free(list);
    PASS();
}

static void test_llist_foreach_count(void) {
    TEST("foreach: callback called exactly size times");
    linked_list_s *list = NULL;
    llist_init(&list, sizeof(int));
    for (int i = 0; i < 50; i++)
        llist_push_back(list, &i);
    ll_call_count = 0;
    llist_foreach(list, ll_count_callback, NULL);
    CHECK(ll_call_count == 50, "call count != size");
    llist_free(list);
    PASS();
}

static void test_llist_foreach_null_args(void) {
    TEST("foreach: NULL args return error");
    linked_list_s *list = NULL;
    llist_init(&list, sizeof(int));
    CHECK(llist_foreach(NULL, ll_count_callback, NULL) == DS_ERR_INVALID_ARGUMENT, "NULL list not caught");
    CHECK(llist_foreach(list, NULL, NULL) == DS_ERR_INVALID_ARGUMENT, "NULL fn not caught");
    llist_free(list);
    PASS();
}


/* ═══════════════════════════════════════════
   MAIN
═══════════════════════════════════════════ */

int main(void) {
    printf("\n=== linked_list_s test suite ===\n\n");

    printf("[ Init / Free ]\n");
    test_init_basic();
    test_init_custom_structs();
    test_free_empty();

    printf("\n[ Push Front ]\n");
    test_push_front_single();
    test_push_front_multiple();
    test_push_front_point();

    printf("\n[ Push Back ]\n");
    test_push_back_single();
    test_push_back_multiple();
    test_push_back_person();

    printf("\n[ Pop Front ]\n");
    test_pop_front_basic();
    test_pop_front_all();
    test_pop_front_empty();
    test_pop_front_null_out();

    printf("\n[ Pop Back ]\n");
    test_pop_back_basic();
    test_pop_back_single();
    test_pop_back_empty();
    test_pop_back_null_out();
    test_pop_back_point();

    printf("\n[ Get ]\n");
    test_get_correct_values();
    test_get_oob();
    test_get_empty();

    printf("\n[ Set ]\n");
    test_set_basic();
    test_set_point();
    test_set_oob();

    printf("\n[ Insert ]\n");
    test_insert_front();
    test_insert_middle();
    test_insert_back();
    test_insert_empty();
    test_insert_oob();

    printf("\n[ Remove ]\n");
    test_remove_front();
    test_remove_middle();
    test_remove_last();
    test_remove_oob();
    test_remove_empty();

    printf("\n[ Edge / Stress ]\n");
    test_push_front_back_interleaved();
    test_large_volume();
    test_single_element_all_ops();
    test_repeated_insert_remove_front();
    test_person_data_integrity();
    test_pop_front_back_alternating();

    printf("\n[ Foreach ]\n");
    test_llist_foreach_basic();
    test_llist_foreach_empty();
    test_llist_foreach_order();
    test_llist_foreach_count();
    test_llist_foreach_null_args();

    printf("\n═══════════════════════════════════════════\n");
    printf("Results: %d / %d passed\n\n", tests_passed, tests_run);

    return (tests_passed == tests_run) ? 0 : 1;
}