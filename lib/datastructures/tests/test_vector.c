#include "datastructures.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
   Custom structs used in tests
───────────────────────────────────────────── */

typedef struct {
    int   x;
    int   y;
} Point;

typedef struct {
    char  name[32];
    int   age;
    float score;
} Person;

typedef struct {
    int   matrix[4][4];
} BigStruct;

/* ─────────────────────────────────────────────
   Utility: make_point / make_person
───────────────────────────────────────────── */
static Point make_point(int x, int y) {
    Point p = { x, y };
    return p;
}

static Person make_person(const char *name, int age, float score) {
    Person p;
    strncpy(p.name, name, sizeof(p.name) - 1);
    p.name[sizeof(p.name) - 1] = '\0';
    p.age   = age;
    p.score = score;
    return p;
}

/* ═══════════════════════════════════════════
   TEST GROUPS
═══════════════════════════════════════════ */

/* ── 1. Init / Free ─────────────────────── */

static void test_init_basic(void) {
    TEST("init: basic initialisation");
    vector_s *v = NULL;
    CHECK(vector_init(&v, sizeof(int)) == DS_OK, "init returned error");
    CHECK(v != NULL,                             "vector is NULL");
    CHECK(vector_size(v)     == 0,               "size != 0");
    CHECK(vector_capacity(v) == 16,              "capacity != 16");
    vector_free(v);
    PASS();
}

static void test_free_null(void) {
    TEST("free: NULL pointer is safe");
    CHECK(vector_free(NULL) == DS_OK, "free(NULL) returned error");
    PASS();
}

static void test_init_custom_structs(void) {
    TEST("init: Point and Person elem sizes");
    vector_s *vp = NULL, *vper = NULL;
    CHECK(vector_init(&vp,   sizeof(Point))  == DS_OK, "Point init failed");
    CHECK(vector_init(&vper, sizeof(Person)) == DS_OK, "Person init failed");
    vector_free(vp);
    vector_free(vper);
    PASS();
}

/* ── 2. Push / Size ─────────────────────── */

static void test_push_ints(void) {
    TEST("push: sequential ints, size grows");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    for (int i = 0; i < 10; i++) {
        CHECK(vector_push(v, &i) == DS_OK, "push failed");
    }
    CHECK(vector_size(v) == 10, "size != 10");
    vector_free(v);
    PASS();
}

static void test_push_triggers_realloc(void) {
    TEST("push: realloc triggered past initial capacity (16)");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    for (int i = 0; i < 32; i++) {
        CHECK(vector_push(v, &i) == DS_OK, "push failed");
    }
    CHECK(vector_size(v)     == 32, "size != 32");
    CHECK(vector_capacity(v) >  16, "capacity did not grow");
    vector_free(v);
    PASS();
}

static void test_push_points(void) {
    TEST("push: custom Point structs");
    vector_s *v = NULL;
    vector_init(&v, sizeof(Point));
    for (int i = 0; i < 5; i++) {
        Point p = make_point(i, i * 2);
        CHECK(vector_push(v, &p) == DS_OK, "push Point failed");
    }
    CHECK(vector_size(v) == 5, "size != 5");
    vector_free(v);
    PASS();
}

static void test_push_persons(void) {
    TEST("push: custom Person structs");
    vector_s *v = NULL;
    vector_init(&v, sizeof(Person));
    Person alice = make_person("Alice", 30, 9.5f);
    Person bob   = make_person("Bob",   25, 8.0f);
    CHECK(vector_push(v, &alice) == DS_OK, "push Alice failed");
    CHECK(vector_push(v, &bob)   == DS_OK, "push Bob failed");
    CHECK(vector_size(v) == 2, "size != 2");
    vector_free(v);
    PASS();
}

static void test_push_big_struct(void) {
    TEST("push: large struct (4x4 int matrix)");
    vector_s *v = NULL;
    vector_init(&v, sizeof(BigStruct));
    BigStruct bs;
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            bs.matrix[r][c] = r * 4 + c;
    CHECK(vector_push(v, &bs) == DS_OK, "push BigStruct failed");
    CHECK(vector_size(v) == 1, "size != 1");
    vector_free(v);
    PASS();
}

/* ── 3. Get ─────────────────────────────── */

static void test_get_correct_value(void) {
    TEST("get: values match what was pushed (int)");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    for (int i = 0; i < 8; i++) vector_push(v, &i);
    for (int i = 0; i < 8; i++) {
        int out = -1;
        CHECK(vector_get(v, (size_t)i, &out) == DS_OK, "get failed");
        CHECK(out == i, "value mismatch");
    }
    vector_free(v);
    PASS();
}

static void test_get_point(void) {
    TEST("get: Point struct fields preserved");
    vector_s *v = NULL;
    vector_init(&v, sizeof(Point));
    Point p = make_point(42, 99);
    vector_push(v, &p);
    Point out = {0, 0};
    CHECK(vector_get(v, 0, &out) == DS_OK, "get failed");
    CHECK(out.x == 42 && out.y == 99, "Point fields wrong");
    vector_free(v);
    PASS();
}

static void test_get_person(void) {
    TEST("get: Person struct fields preserved");
    vector_s *v = NULL;
    vector_init(&v, sizeof(Person));
    Person p = make_person("Charlie", 40, 7.7f);
    vector_push(v, &p);
    Person out;
    memset(&out, 0, sizeof(out));
    CHECK(vector_get(v, 0, &out) == DS_OK, "get failed");
    CHECK(strcmp(out.name, "Charlie") == 0, "name mismatch");
    CHECK(out.age == 40,                    "age mismatch");
    vector_free(v);
    PASS();
}

static void test_get_out_of_bounds(void) {
    TEST("get: out-of-bounds returns DS_ERR_OUT_OF_BOUNDS");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    int val = 1;
    vector_push(v, &val);
    int out = 0;
    CHECK(vector_get(v, 5, &out) == DS_ERR_OUT_OF_BOUNDS, "expected OOB error");
    vector_free(v);
    PASS();
}

static void test_get_empty_vector(void) {
    TEST("get: empty vector returns DS_ERR_OUT_OF_BOUNDS");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    int out = 0;
    CHECK(vector_get(v, 0, &out) == DS_ERR_OUT_OF_BOUNDS, "expected OOB error");
    vector_free(v);
    PASS();
}

/* ── 4. Set ─────────────────────────────── */

static void test_set_basic(void) {
    TEST("set: overwrites existing value");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    int val = 10; vector_push(v, &val);
    val = 99;
    CHECK(vector_set(v, &val, 0) == DS_OK, "set failed");
    int out = 0;
    vector_get(v, 0, &out);
    CHECK(out == 99, "value not updated");
    vector_free(v);
    PASS();
}

static void test_set_point(void) {
    TEST("set: overwrites Point struct");
    vector_s *v = NULL;
    vector_init(&v, sizeof(Point));
    Point p = make_point(1, 2); vector_push(v, &p);
    Point newp = make_point(7, 8);
    CHECK(vector_set(v, &newp, 0) == DS_OK, "set failed");
    Point out = {0, 0};
    vector_get(v, 0, &out);
    CHECK(out.x == 7 && out.y == 8, "Point not updated");
    vector_free(v);
    PASS();
}

static void test_set_out_of_bounds(void) {
    TEST("set: out-of-bounds returns DS_ERR_OUT_OF_BOUNDS");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    int val = 5; vector_push(v, &val);
    CHECK(vector_set(v, &val, 10) == DS_ERR_OUT_OF_BOUNDS, "expected OOB error");
    vector_free(v);
    PASS();
}

/* ── 5. Pop ─────────────────────────────── */

static void test_pop_basic(void) {
    TEST("pop: returns last element, shrinks size");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    int a = 1, b = 2, c = 3;
    vector_push(v, &a); vector_push(v, &b); vector_push(v, &c);
    int out = 0;
    CHECK(vector_pop(v, &out) == DS_OK, "pop failed");
    CHECK(out == 3,              "wrong value popped");
    CHECK(vector_size(v) == 2,   "size not decremented");
    vector_free(v);
    PASS();
}

static void test_pop_all(void) {
    TEST("pop: pop all elements one by one");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    for (int i = 0; i < 5; i++) vector_push(v, &i);
    for (int i = 4; i >= 0; i--) {
        int out = -1;
        CHECK(vector_pop(v, &out) == DS_OK, "pop failed");
        CHECK(out == i, "wrong pop order");
    }
    CHECK(vector_size(v) == 0, "size should be 0");
    vector_free(v);
    PASS();
}

static void test_pop_empty(void) {
    TEST("pop: empty vector returns DS_ERR_EMPTY");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    int out = 0;
    CHECK(vector_pop(v, &out) == DS_ERR_EMPTY, "expected EMPTY error");
    vector_free(v);
    PASS();
}

static void test_pop_point(void) {
    TEST("pop: Point struct value correct");
    vector_s *v = NULL;
    vector_init(&v, sizeof(Point));
    Point p = make_point(11, 22);
    vector_push(v, &p);
    Point out = {0, 0};
    CHECK(vector_pop(v, &out) == DS_OK, "pop failed");
    CHECK(out.x == 11 && out.y == 22,   "Point fields wrong");
    vector_free(v);
    PASS();
}

/* ── 6. Insert ──────────────────────────── */

static void test_insert_front(void) {
    TEST("insert: at index 0 shifts all elements right");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    for (int i = 1; i <= 4; i++) vector_push(v, &i);  /* [1,2,3,4] */
    int val = 99;
    CHECK(vector_insert(v, &val, 0) == DS_OK, "insert failed");
    /* expected: [99,1,2,3,4] */
    int out = 0;
    vector_get(v, 0, &out); CHECK(out == 99, "front value wrong");
    vector_get(v, 1, &out); CHECK(out == 1,  "shifted value wrong");
    CHECK(vector_size(v) == 5, "size wrong");
    vector_free(v);
    PASS();
}

static void test_insert_middle(void) {
    TEST("insert: in the middle shifts right elements only");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    int vals[] = {1, 2, 4, 5};
    for (int i = 0; i < 4; i++) vector_push(v, &vals[i]);
    int val = 3;
    CHECK(vector_insert(v, &val, 2) == DS_OK, "insert failed");
    /* expected: [1,2,3,4,5] */
    for (int i = 0; i < 5; i++) {
        int out = 0;
        vector_get(v, (size_t)i, &out);
        CHECK(out == i + 1, "sequence broken after insert");
    }
    vector_free(v);
    PASS();
}

static void test_insert_end(void) {
    TEST("insert: at index == size behaves like push");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    int a = 1, b = 2;
    vector_push(v, &a);
    CHECK(vector_insert(v, &b, 1) == DS_OK, "insert at end failed");
    int out = 0;
    vector_get(v, 1, &out);
    CHECK(out == 2, "value wrong");
    vector_free(v);
    PASS();
}

static void test_insert_out_of_bounds(void) {
    TEST("insert: beyond size returns DS_ERR_OUT_OF_BOUNDS");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    int val = 1; vector_push(v, &val);
    CHECK(vector_insert(v, &val, 5) == DS_ERR_OUT_OF_BOUNDS, "expected OOB error");
    vector_free(v);
    PASS();
}

static void test_insert_point(void) {
    TEST("insert: Point struct inserted correctly");
    vector_s *v = NULL;
    vector_init(&v, sizeof(Point));
    Point a = make_point(1, 1), b = make_point(3, 3), mid = make_point(2, 2);
    vector_push(v, &a); vector_push(v, &b);
    CHECK(vector_insert(v, &mid, 1) == DS_OK, "insert failed");
    Point out = {0, 0};
    vector_get(v, 1, &out);
    CHECK(out.x == 2 && out.y == 2, "inserted Point wrong");
    vector_free(v);
    PASS();
}

/* ── 7. Remove ──────────────────────────── */

static void test_remove_front(void) {
    TEST("remove: index 0 shifts all elements left");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    for (int i = 0; i < 5; i++) vector_push(v, &i); /* [0,1,2,3,4] */
    CHECK(vector_remove(v, 0) == DS_OK, "remove failed");
    int out = 0;
    vector_get(v, 0, &out);
    CHECK(out == 1,              "front element wrong after remove");
    CHECK(vector_size(v) == 4,   "size wrong");
    vector_free(v);
    PASS();
}

static void test_remove_middle(void) {
    TEST("remove: middle index compacts correctly");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    int vals[] = {1, 2, 99, 3, 4};
    for (int i = 0; i < 5; i++) vector_push(v, &vals[i]);
    CHECK(vector_remove(v, 2) == DS_OK, "remove failed");
    for (int i = 0; i < 4; i++) {
        int out = 0;
        vector_get(v, (size_t)i, &out);
        CHECK(out == i + 1, "sequence broken after remove");
    }
    vector_free(v);
    PASS();
}

static void test_remove_last(void) {
    TEST("remove: last element decrements size only");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    int a = 1, b = 2;
    vector_push(v, &a); vector_push(v, &b);
    CHECK(vector_remove(v, 1) == DS_OK, "remove last failed");
    CHECK(vector_size(v) == 1,          "size wrong");
    vector_free(v);
    PASS();
}

static void test_remove_out_of_bounds(void) {
    TEST("remove: out-of-bounds returns DS_ERR_OUT_OF_BOUNDS");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    int val = 1; vector_push(v, &val);
    CHECK(vector_remove(v, 5) == DS_ERR_OUT_OF_BOUNDS, "expected OOB error");
    vector_free(v);
    PASS();
}

static void test_remove_empty(void) {
    TEST("remove: empty vector returns DS_ERR_OUT_OF_BOUNDS");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    CHECK(vector_remove(v, 0) == DS_ERR_OUT_OF_BOUNDS, "expected OOB error");
    vector_free(v);
    PASS();
}

/* ── 8. Edge / stress cases ─────────────── */

static void test_push_pop_interleaved(void) {
    TEST("edge: interleaved push and pop stays consistent");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    int val, out;
    val = 1; vector_push(v, &val);
    val = 2; vector_push(v, &val);
    vector_pop(v, &out); CHECK(out == 2, "wrong pop");
    val = 3; vector_push(v, &val);
    vector_pop(v, &out); CHECK(out == 3, "wrong pop");
    vector_pop(v, &out); CHECK(out == 1, "wrong pop");
    CHECK(vector_size(v) == 0, "size should be 0");
    vector_free(v);
    PASS();
}

static void test_large_volume(void) {
    TEST("edge: 10 000 ints pushed then verified");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    for (int i = 0; i < 10000; i++) {
        CHECK(vector_push(v, &i) == DS_OK, "push failed");
    }
    CHECK(vector_size(v) == 10000, "size wrong");
    for (int i = 0; i < 10000; i++) {
        int out = -1;
        vector_get(v, (size_t)i, &out);
        CHECK(out == i, "value mismatch at large volume");
    }
    vector_free(v);
    PASS();
}

/* ── Foreach ────────────────────────────── */

static void vec_sum_callback(const void *element, size_t index, void *user_data) {
    (void)index;
    int *sum = user_data;
    *sum += *(const int *)element;
}

static int vec_call_count = 0;
static void vec_count_callback(const void *element, size_t index, void *user_data) {
    (void)element; (void)index; (void)user_data;
    vec_call_count++;
}

static void test_vector_foreach_basic(void) {
    TEST("foreach: visits all elements, sum matches");
    vector_s *vec = NULL;
    vector_init(&vec, sizeof(int));
    int expected = 0;
    for (int i = 1; i <= 10; i++) {
        vector_push(vec, &i);
        expected += i;
    }
    int sum = 0;
    CHECK(vector_foreach(vec, vec_sum_callback, &sum) == DS_OK, "foreach failed");
    CHECK(sum == expected, "sum mismatch");
    vector_free(vec);
    PASS();
}

static void test_vector_foreach_empty(void) {
    TEST("foreach: empty vector, callback never called");
    vector_s *vec = NULL;
    vector_init(&vec, sizeof(int));
    vec_call_count = 0;
    CHECK(vector_foreach(vec, vec_count_callback, NULL) == DS_OK, "foreach failed");
    CHECK(vec_call_count == 0, "callback called on empty vec");
    vector_free(vec);
    PASS();
}

static int order_expected = 0;
static int order_correct = 1;
static void vec_order_callback(const void *element, size_t index, void *user_data) {
    (void)user_data;
    if (*(const int *)element != order_expected || (int)index != order_expected)
        order_correct = 0;
    order_expected++;
}

static void test_vector_foreach_order(void) {
    TEST("foreach: visits elements in index order");
    vector_s *vec = NULL;
    vector_init(&vec, sizeof(int));
    for (int i = 0; i < 5; i++)
        vector_push(vec, &i);
    order_expected = 0;
    order_correct = 1;
    vector_foreach(vec, vec_order_callback, NULL);
    CHECK(order_correct == 1, "elements visited out of order");
    vector_free(vec);
    PASS();
}


static void test_vector_foreach_count(void) {
    TEST("foreach: callback called exactly size times");
    vector_s *vec = NULL;
    vector_init(&vec, sizeof(int));
    for (int i = 0; i < 100; i++)
        vector_push(vec, &i);
    vec_call_count = 0;
    vector_foreach(vec, vec_count_callback, NULL);
    CHECK(vec_call_count == 100, "call count != size");
    vector_free(vec);
    PASS();
}

static void test_vector_foreach_null_args(void) {
    TEST("foreach: NULL args return error");
    vector_s *vec = NULL;
    vector_init(&vec, sizeof(int));
    CHECK(vector_foreach(NULL, vec_count_callback, NULL) == DS_ERR_INVALID_ARGUMENT, "NULL vec not caught");
    CHECK(vector_foreach(vec, NULL, NULL) == DS_ERR_INVALID_ARGUMENT, "NULL fn not caught");
    vector_free(vec);
    PASS();
}


static void test_single_element_all_ops(void) {
    TEST("edge: single-element vector survives get/set/pop/remove");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    int val = 42; vector_push(v, &val);

    int out = 0;
    CHECK(vector_get(v, 0, &out) == DS_OK && out == 42, "get failed");

    val = 77;
    CHECK(vector_set(v, &val, 0) == DS_OK, "set failed");
    vector_get(v, 0, &out);
    CHECK(out == 77, "set value wrong");

    CHECK(vector_pop(v, &out) == DS_OK && out == 77, "pop failed");
    CHECK(vector_size(v) == 0, "size should be 0 after pop");
    vector_free(v);
    PASS();
}

static void test_insert_into_empty(void) {
    TEST("edge: insert at index 0 into empty vector");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    int val = 5;
    CHECK(vector_insert(v, &val, 0) == DS_OK, "insert into empty failed");
    CHECK(vector_size(v) == 1, "size wrong");
    int out = 0;
    vector_get(v, 0, &out);
    CHECK(out == 5, "value wrong");
    vector_free(v);
    PASS();
}

static void test_repeated_insert_remove_front(void) {
    TEST("edge: repeated insert/remove at front preserves data");
    vector_s *v = NULL;
    vector_init(&v, sizeof(int));
    for (int i = 0; i < 8; i++) {
        int val = i;
        vector_push(v, &val);
    }
    /* insert 99 at front, then immediately remove it, 50 times */
    for (int i = 0; i < 50; i++) {
        int val = 99;
        vector_insert(v, &val, 0);
        vector_remove(v, 0);
    }
    CHECK(vector_size(v) == 8, "size corrupted");
    for (int i = 0; i < 8; i++) {
        int out = -1;
        vector_get(v, (size_t)i, &out);
        CHECK(out == i, "data corrupted after repeated insert/remove");
    }
    vector_free(v);
    PASS();
}

static void test_person_data_integrity_after_realloc(void) {
    TEST("edge: Person data intact after multiple reallocations");
    vector_s *v = NULL;
    vector_init(&v, sizeof(Person));
    /* push 100 persons — will trigger several reallocations */
    for (int i = 0; i < 100; i++) {
        char name[32];
        snprintf(name, sizeof(name), "Person%d", i);
        Person p = make_person(name, i, (float)i * 0.5f);
        CHECK(vector_push(v, &p) == DS_OK, "push failed");
    }
    for (int i = 0; i < 100; i++) {
        Person out;
        memset(&out, 0, sizeof(out));
        vector_get(v, (size_t)i, &out);
        CHECK(out.age == i, "age corrupted after realloc");
    }
    vector_free(v);
    PASS();
}

/* ═══════════════════════════════════════════
   MAIN
═══════════════════════════════════════════ */

int main(void) {
    printf("\n=== vector_s test suite ===\n\n");

    printf("[ Init / Free ]\n");
    test_init_basic();
    test_free_null();
    test_init_custom_structs();

    printf("\n[ Push / Size ]\n");
    test_push_ints();
    test_push_triggers_realloc();
    test_push_points();
    test_push_persons();
    test_push_big_struct();

    printf("\n[ Get ]\n");
    test_get_correct_value();
    test_get_point();
    test_get_person();
    test_get_out_of_bounds();
    test_get_empty_vector();

    printf("\n[ Set ]\n");
    test_set_basic();
    test_set_point();
    test_set_out_of_bounds();

    printf("\n[ Pop ]\n");
    test_pop_basic();
    test_pop_all();
    test_pop_empty();
    test_pop_point();

    printf("\n[ Insert ]\n");
    test_insert_front();
    test_insert_middle();
    test_insert_end();
    test_insert_out_of_bounds();
    test_insert_point();

    printf("\n[ Remove ]\n");
    test_remove_front();
    test_remove_middle();
    test_remove_last();
    test_remove_out_of_bounds();
    test_remove_empty();

    printf("\n[ Edge / Stress ]\n");
    test_push_pop_interleaved();
    test_large_volume();
    test_single_element_all_ops();
    test_insert_into_empty();
    test_repeated_insert_remove_front();
    test_person_data_integrity_after_realloc();

    printf("\n[ Foreach ]\n");
    test_vector_foreach_basic();
    test_vector_foreach_empty();
    test_vector_foreach_order();
    test_vector_foreach_count();
    test_vector_foreach_null_args();

    printf("\n═══════════════════════════════════════════\n");
    printf("Results: %d / %d passed\n\n", tests_passed, tests_run);

    return (tests_passed == tests_run) ? 0 : 1;
}