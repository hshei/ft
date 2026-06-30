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
        printf("  %-60s", name); \
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

static Point make_point(int x, int y) { Point p = {x, y}; return p; }

/* ═══════════════════════════════════════════
   TEST GROUPS
═══════════════════════════════════════════ */

/* ── 1. Init / Free ─────────────────────── */

static void test_init_basic(void) {
    TEST("init: basic initialisation");
    hashset_s *hs = NULL;
    CHECK(hs_init(&hs, sizeof(int), 16) == DS_OK, "init failed");
    CHECK(hs != NULL,               "hs is NULL");
    CHECK(hs->size == 0,            "size != 0");
    CHECK(hs->bucket_count == 16,   "bucket_count != 16");
    CHECK(hs->buckets != NULL,      "buckets is NULL");
    hs_free(hs);
    PASS();
}

static void test_init_all_buckets_null(void) {
    TEST("init: all buckets initialised to NULL");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 16);
    for (size_t i = 0; i < 16; i++)
        CHECK(hs->buckets[i] == NULL, "bucket not NULL after init");
    hs_free(hs);
    PASS();
}

static void test_free_empty(void) {
    TEST("free: empty hashset is safe");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 16);
    CHECK(hs_free(hs) == DS_OK, "free empty failed");
    PASS();
}

static void test_free_null(void) {
    TEST("free: NULL hashset is safe");
    CHECK(hs_free(NULL) == DS_OK, "free NULL failed");
    PASS();
}

/* ── 2. Insert / Contains ──────────────── */

static void test_insert_and_contains_int(void) {
    TEST("insert/contains: single int key");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 16);
    int key = 42;
    CHECK(hs_insert(hs, &key) == DS_OK, "insert failed");
    CHECK(hs->size == 1, "size != 1");
    bool result = 0;
    CHECK(hs_contains(hs, &key, &result) == DS_OK, "contains failed");
    CHECK(result == 1, "key not found");
    hs_free(hs);
    PASS();
}

static void test_insert_multiple(void) {
    TEST("insert/contains: multiple int keys");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 16);
    for (int i = 0; i < 10; i++)
        CHECK(hs_insert(hs, &i) == DS_OK, "insert failed");
    CHECK(hs->size == 10, "size != 10");
    for (int i = 0; i < 10; i++) {
        bool result = 0;
        CHECK(hs_contains(hs, &i, &result) == DS_OK, "contains failed");
        CHECK(result == 1, "key not found");
    }
    hs_free(hs);
    PASS();
}

static void test_insert_duplicate(void) {
    TEST("insert: duplicate key does not increase size");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 16);
    int key = 7;
    hs_insert(hs, &key);
    hs_insert(hs, &key);
    hs_insert(hs, &key);
    CHECK(hs->size == 1, "size should still be 1");
    bool result = 0;
    hs_contains(hs, &key, &result);
    CHECK(result == 1, "key not found");
    hs_free(hs);
    PASS();
}

static void test_insert_struct_key(void) {
    TEST("insert/contains: Point struct as key");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(Point), 16);
    Point p1 = make_point(3, 7);
    Point p2 = make_point(10, 20);
    CHECK(hs_insert(hs, &p1) == DS_OK, "insert p1 failed");
    CHECK(hs_insert(hs, &p2) == DS_OK, "insert p2 failed");
    CHECK(hs->size == 2, "size != 2");
    bool result = 0;
    hs_contains(hs, &p1, &result);
    CHECK(result == 1, "p1 not found");
    hs_contains(hs, &p2, &result);
    CHECK(result == 1, "p2 not found");
    hs_free(hs);
    PASS();
}

static void test_contains_missing_key(void) {
    TEST("contains: missing key returns 0");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 16);
    int key = 42;
    bool result = -1;
    CHECK(hs_contains(hs, &key, &result) == DS_OK, "contains failed");
    CHECK(result == 0, "expected 0 for missing key");
    hs_free(hs);
    PASS();
}

static void test_contains_after_other_inserts(void) {
    TEST("contains: missing key with other keys present");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 16);
    int k1 = 1, k2 = 2;
    hs_insert(hs, &k1);
    hs_insert(hs, &k2);
    int missing = 999;
    bool result = false;
    hs_contains(hs, &missing, &result);
    CHECK(result == false, "expected 0 for missing key");
    hs_free(hs);
    PASS();
}

/* ── 3. Remove ──────────────────────────── */

static void test_remove_basic(void) {
    TEST("remove: existing key removed, size decremented");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 16);
    int key = 1;
    hs_insert(hs, &key);
    CHECK(hs_remove(hs, &key) == DS_OK, "remove failed");
    CHECK(hs->size == 0, "size != 0");
    bool result = -1;
    hs_contains(hs, &key, &result);
    CHECK(result == 0, "key still present after remove");
    hs_free(hs);
    PASS();
}

static void test_remove_missing_key(void) {
    TEST("remove: missing key returns DS_ERR_NOT_FOUND");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 16);
    int key = 99;
    CHECK(hs_remove(hs, &key) == DS_ERR_NOT_FOUND, "expected NOT_FOUND");
    hs_free(hs);
    PASS();
}

static void test_remove_head_of_chain(void) {
    TEST("remove: head of chain, rest of chain intact");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 1);
    int k1 = 1, k2 = 2, k3 = 3;
    hs_insert(hs, &k1);
    hs_insert(hs, &k2);
    hs_insert(hs, &k3);
    CHECK(hs_remove(hs, &k3) == DS_OK, "remove head failed");
    CHECK(hs->size == 2, "size wrong");
    bool result = 0;
    hs_contains(hs, &k1, &result);
    CHECK(result == 1, "k1 lost after head remove");
    hs_contains(hs, &k2, &result);
    CHECK(result == 1, "k2 lost after head remove");
    hs_free(hs);
    PASS();
}

static void test_remove_middle_of_chain(void) {
    TEST("remove: middle of chain, neighbours intact");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 1);
    int k1 = 1, k2 = 2, k3 = 3;
    hs_insert(hs, &k1);
    hs_insert(hs, &k2);
    hs_insert(hs, &k3);
    CHECK(hs_remove(hs, &k2) == DS_OK, "remove middle failed");
    CHECK(hs->size == 2, "size wrong");
    bool result = 0;
    hs_contains(hs, &k1, &result);
    CHECK(result == 1, "k1 lost");
    hs_contains(hs, &k3, &result);
    CHECK(result == 1, "k3 lost");
    hs_contains(hs, &k2, &result);
    CHECK(result == 0, "k2 still present");
    hs_free(hs);
    PASS();
}

static void test_remove_then_reinsert(void) {
    TEST("remove: removed key can be reinserted");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 16);
    int key = 5;
    hs_insert(hs, &key);
    hs_remove(hs, &key);
    CHECK(hs_insert(hs, &key) == DS_OK, "reinsert failed");
    bool result = 0;
    hs_contains(hs, &key, &result);
    CHECK(result == 1, "key not found after reinsert");
    CHECK(hs->size == 1, "size wrong after reinsert");
    hs_free(hs);
    PASS();
}

/* ── 4. Collisions ──────────────────────── */

static void test_collision_chaining(void) {
    TEST("collision: bucket_count=1 forces all into one chain");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 1);
    for (int i = 0; i < 10; i++)
        CHECK(hs_insert(hs, &i) == DS_OK, "insert failed");
    CHECK(hs->size == 10, "size wrong");
    for (int i = 0; i < 10; i++) {
        bool result = 0;
        hs_contains(hs, &i, &result);
        CHECK(result == 1, "key not found in chain");
    }
    hs_free(hs);
    PASS();
}

static void test_collision_remove_all_in_chain(void) {
    TEST("collision: remove all entries from single bucket");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 1);
    for (int i = 0; i < 5; i++)
        hs_insert(hs, &i);
    for (int i = 0; i < 5; i++)
        CHECK(hs_remove(hs, &i) == DS_OK, "remove from chain failed");
    CHECK(hs->size == 0, "size != 0");
    CHECK(hs->buckets[0] == NULL, "bucket not NULL after all removed");
    hs_free(hs);
    PASS();
}

/* ── 5. Rehash ──────────────────────────── */

static void test_rehash_triggers_and_preserves_data(void) {
    TEST("rehash: triggers on load > 0.75, all data preserved");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 4);
    for (int i = 0; i < 20; i++)
        CHECK(hs_insert(hs, &i) == DS_OK, "insert failed");
    CHECK(hs->bucket_count > 4, "bucket_count did not grow");
    CHECK(hs->size == 20, "size wrong after rehash");
    for (int i = 0; i < 20; i++) {
        bool result = 0;
        hs_contains(hs, &i, &result);
        CHECK(result == 1, "key lost after rehash");
    }
    hs_free(hs);
    PASS();
}

static void test_rehash_multiple_times(void) {
    TEST("rehash: triggers multiple times, data intact");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 2);
    for (int i = 0; i < 100; i++)
        CHECK(hs_insert(hs, &i) == DS_OK, "insert failed");
    CHECK(hs->bucket_count > 2, "bucket_count did not grow");
    CHECK(hs->size == 100, "size wrong");
    for (int i = 0; i < 100; i++) {
        bool result = 0;
        hs_contains(hs, &i, &result);
        CHECK(result == true, "key lost after multiple rehashes");
    }
    hs_free(hs);
    PASS();
}

static void test_rehash_remove_after_rehash(void) {
    TEST("rehash: remove works correctly after rehash");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 4);
    for (int i = 0; i < 20; i++)
        hs_insert(hs, &i);
    for (int i = 0; i < 20; i += 2)
        CHECK(hs_remove(hs, &i) == DS_OK, "remove after rehash failed");
    CHECK(hs->size == 10, "size wrong after removes");
    for (int i = 0; i < 20; i++) {
        bool result = -1;
        hs_contains(hs, &i, &result);
        if (i % 2 == 0)
            CHECK(result == 0, "even key still present");
        else
            CHECK(result == 1, "odd key missing");
    }
    hs_free(hs);
    PASS();
}

/* ── 6. Edge / Stress ───────────────────── */

static void test_large_volume(void) {
    TEST("edge: 1000 entries inserted and verified");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 64);
    for (int i = 0; i < 1000; i++)
        CHECK(hs_insert(hs, &i) == DS_OK, "insert failed");
    CHECK(hs->size == 1000, "size wrong");
    for (int i = 0; i < 1000; i++) {
        bool result = 0;
        hs_contains(hs, &i, &result);
        CHECK(result == 1, "key not found");
    }
    hs_free(hs);
    PASS();
}

static void test_insert_remove_interleaved(void) {
    TEST("edge: interleaved insert and remove");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 16);
    int k1 = 1, k2 = 2, k3 = 3;
    hs_insert(hs, &k1);
    hs_insert(hs, &k2);
    hs_remove(hs, &k1);
    hs_insert(hs, &k3);
    hs_remove(hs, &k2);
    CHECK(hs->size == 1, "size wrong");
    bool result = 0;
    hs_contains(hs, &k3, &result);
    CHECK(result == 1, "k3 not found");
    hs_contains(hs, &k1, &result);
    CHECK(result == 0, "k1 still present");
    hs_contains(hs, &k2, &result);
    CHECK(result == 0, "k2 still present");
    hs_free(hs);
    PASS();
}

static void test_duplicates_stress(void) {
    TEST("edge: 100 inserts of same key, size stays 1");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 16);
    int key = 42;
    for (int i = 0; i < 100; i++)
        hs_insert(hs, &key);
    CHECK(hs->size == 1, "size should be 1");
    hs_free(hs);
    PASS();
}

static void test_free_with_collisions(void) {
    TEST("edge: free with multiple collisions doesn't leak");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 1);
    for (int i = 0; i < 20; i++)
        hs_insert(hs, &i);
    CHECK(hs_free(hs) == DS_OK, "free with collisions failed");
    PASS();
}

/* ── 7. Foreach ─────────────────────────── */

static void sum_callback(const void *key, void *user_data) {
    int *sum = user_data;
    *sum += *(const int *)key;
}

static int call_count = 0;
static void count_callback(const void *key, void *user_data) {
    (void)key; (void)user_data;
    call_count++;
}

static void test_foreach_basic(void) {
    TEST("foreach: visits all keys, sum matches");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 16);
    int expected_sum = 0;
    for (int i = 1; i <= 5; i++) {
        hs_insert(hs, &i);
        expected_sum += i;
    }
    int sum = 0;
    CHECK(hs_foreach(hs, sum_callback, &sum) == DS_OK, "foreach failed");
    CHECK(sum == expected_sum, "sum mismatch");
    hs_free(hs);
    PASS();
}

static void test_foreach_empty(void) {
    TEST("foreach: empty hashset, callback never called");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 16);
    call_count = 0;
    CHECK(hs_foreach(hs, count_callback, NULL) == DS_OK, "foreach failed");
    CHECK(call_count == 0, "callback called on empty set");
    hs_free(hs);
    PASS();
}

static void test_foreach_count_matches_size(void) {
    TEST("foreach: callback called exactly size times");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 16);
    for (int i = 0; i < 20; i++)
        hs_insert(hs, &i);
    call_count = 0;
    hs_foreach(hs, count_callback, NULL);
    CHECK(call_count == 20, "call count != size");
    hs_free(hs);
    PASS();
}

static void test_foreach_with_collisions(void) {
    TEST("foreach: single bucket, all keys visited");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 1);
    int expected_sum = 0;
    for (int i = 0; i < 10; i++) {
        hs_insert(hs, &i);
        expected_sum += i;
    }
    int sum = 0;
    hs_foreach(hs, sum_callback, &sum);
    CHECK(sum == expected_sum, "sum mismatch with collisions");
    hs_free(hs);
    PASS();
}

static void test_foreach_null_args(void) {
    TEST("foreach: NULL hashset or NULL fn returns error");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 16);
    CHECK(hs_foreach(NULL, count_callback, NULL) == DS_ERR_INVALID_ARGUMENT, "NULL hs not caught");
    CHECK(hs_foreach(hs, NULL, NULL) == DS_ERR_INVALID_ARGUMENT, "NULL fn not caught");
    hs_free(hs);
    PASS();
}

/* ── 8. Get Keys ────────────────────────── */

static void test_get_keys_basic(void) {
    TEST("get_keys: all keys returned in vector");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 16);
    for (int i = 0; i < 5; i++)
        hs_insert(hs, &i);
    vector_s *keys = NULL;
    vector_init(&keys, sizeof(int));
    CHECK(hs_get_keys(hs, keys) == DS_OK, "get_keys failed");
    CHECK(keys->size == 5, "key count mismatch");
    for (int i = 0; i < 5; i++) {
        int key;
        vector_get(keys, i, &key);
        bool found = false;
        hs_contains(hs, &key, &found);
        CHECK(found == true, "returned key not in hashset");
    }
    vector_free(keys);
    hs_free(hs);
    PASS();
}

static void test_get_keys_empty(void) {
    TEST("get_keys: empty hashset returns empty vector");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 16);
    vector_s *keys = NULL;
    vector_init(&keys, sizeof(int));
    CHECK(hs_get_keys(hs, keys) == DS_OK, "get_keys failed");
    CHECK(keys->size == 0, "vector should be empty");
    vector_free(keys);
    hs_free(hs);
    PASS();
}

static void test_get_keys_with_collisions(void) {
    TEST("get_keys: single bucket, all keys collected");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 1);
    for (int i = 0; i < 10; i++)
        hs_insert(hs, &i);
    vector_s *keys = NULL;
    vector_init(&keys, sizeof(int));
    hs_get_keys(hs, keys);
    CHECK(keys->size == 10, "key count mismatch");
    vector_free(keys);
    hs_free(hs);
    PASS();
}

static void test_get_keys_after_removes(void) {
    TEST("get_keys: only surviving keys returned");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 16);
    for (int i = 0; i < 10; i++)
        hs_insert(hs, &i);
    for (int i = 0; i < 5; i++)
        hs_remove(hs, &i);
    vector_s *keys = NULL;
    vector_init(&keys, sizeof(int));
    hs_get_keys(hs, keys);
    CHECK(keys->size == 5, "key count mismatch");
    for (size_t i = 0; i < keys->size; i++) {
        int key;
        vector_get(keys, i, &key);
        CHECK(key >= 5, "removed key still present");
    }
    vector_free(keys);
    hs_free(hs);
    PASS();
}

static void test_get_keys_null_args(void) {
    TEST("get_keys: NULL args return error");
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), 16);
    vector_s *keys = NULL;
    vector_init(&keys, sizeof(int));
    CHECK(hs_get_keys(NULL, keys) == DS_ERR_INVALID_ARGUMENT, "NULL hs not caught");
    CHECK(hs_get_keys(hs, NULL) == DS_ERR_INVALID_ARGUMENT, "NULL out not caught");
    vector_free(keys);
    hs_free(hs);
    PASS();
}

/* ═══════════════════════════════════════════
   MAIN
═══════════════════════════════════════════ */

int main(void) {
    printf("\n=== hashset_s test suite ===\n\n");

    printf("[ Init / Free ]\n");
    test_init_basic();
    test_init_all_buckets_null();
    test_free_empty();
    test_free_null();

    printf("\n[ Insert / Contains ]\n");
    test_insert_and_contains_int();
    test_insert_multiple();
    test_insert_duplicate();
    test_insert_struct_key();
    test_contains_missing_key();
    test_contains_after_other_inserts();

    printf("\n[ Remove ]\n");
    test_remove_basic();
    test_remove_missing_key();
    test_remove_head_of_chain();
    test_remove_middle_of_chain();
    test_remove_then_reinsert();

    printf("\n[ Collisions ]\n");
    test_collision_chaining();
    test_collision_remove_all_in_chain();

    printf("\n[ Rehash ]\n");
    test_rehash_triggers_and_preserves_data();
    test_rehash_multiple_times();
    test_rehash_remove_after_rehash();

    printf("\n[ Edge / Stress ]\n");
    test_large_volume();
    test_insert_remove_interleaved();
    test_duplicates_stress();
    test_free_with_collisions();

    printf("\n[ Foreach ]\n");
    test_foreach_basic();
    test_foreach_empty();
    test_foreach_count_matches_size();
    test_foreach_with_collisions();
    test_foreach_null_args();

    printf("\n[ Get Keys ]\n");
    test_get_keys_basic();
    test_get_keys_empty();
    test_get_keys_with_collisions();
    test_get_keys_after_removes();
    test_get_keys_null_args();


    printf("\n═══════════════════════════════════════════\n");
    printf("Results: %d / %d passed\n\n", tests_passed, tests_run);

    return (tests_passed == tests_run) ? 0 : 1;
}
