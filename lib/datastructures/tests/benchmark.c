#include "vector.h"
#include "linked_list.h"
#include "hashmap.h"
#include "hashset.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

/* ─────────────────────────────────────────────
   Timer helpers
───────────────────────────────────────────── */

static struct timespec timer_start(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t;
}

static double timer_end(struct timespec start) {
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    return (end.tv_sec - start.tv_sec) * 1000.0
         + (end.tv_nsec - start.tv_nsec) / 1.0e6;  /* milliseconds */
}

#define STR(x) #x
#define BENCH(label, N, code) \
    do { \
        struct timespec _t = timer_start(); \
        code; \
        double _ms = timer_end(_t); \
        printf("  %-55s %8.3f ms  (%zu ops)\n", label, _ms, (size_t)(N)); \
    } while(0)

/* ─────────────────────────────────────────────
   Sizes
───────────────────────────────────────────── */

#define SMALL  1000
#define MEDIUM 10000
#define LARGE  100000

/* ═══════════════════════════════════════════
   VECTOR BENCHMARKS
═══════════════════════════════════════════ */

static void bench_vector(void) {
    printf("\n[ Vector ]\n");
    vector_s *v = NULL;

    /* sequential push */
    vector_init(&v, sizeof(int));
    BENCH("push_back x100k", LARGE, {
        for (int i = 0; i < LARGE; i++)
            vector_push(v, &i);
    });

    /* sequential get */
    BENCH("sequential get x100k", LARGE, {
        int out;
        for (size_t i = 0; i < LARGE; i++)
            vector_get(v, i, &out);
    });

    /* random get */
    BENCH("random get x100k", LARGE, {
        int out;
        for (size_t i = 0; i < LARGE; i++)
            vector_get(v, (size_t)(rand() % LARGE), &out);
    });

    /* middle insert — expensive, use smaller N */
    vector_free(v);
    vector_init(&v, sizeof(int));
    for (int i = 0; i < MEDIUM; i++) vector_push(v, &i);
    BENCH("middle insert x10k", MEDIUM, {
        int val = 99;
        for (int i = 0; i < MEDIUM; i++)
            vector_insert(v, &val, v->size / 2);
    });

    /* sequential pop */
    BENCH("pop x" STR(MEDIUM) " (draining)", (size_t)v->size, {
        int out;
        while (v->size > 0)
            vector_pop(v, &out);
    });

    vector_free(v);
}

/* ═══════════════════════════════════════════
   LINKED LIST BENCHMARKS
═══════════════════════════════════════════ */

static void bench_linked_list(void) {
    printf("\n[ Linked List ]\n");
    linked_list_s *l = NULL;

    /* push back */
    llist_init(&l, sizeof(int));
    BENCH("push_back x100k", LARGE, {
        for (int i = 0; i < LARGE; i++)
            llist_push_back(l, &i);
    });

    /* push front */
    BENCH("push_front x10k", MEDIUM, {
        for (int i = 0; i < MEDIUM; i++)
            llist_push_front(l, &i);
    });

    /* sequential get — O(n) per call, use small N */
    BENCH("sequential get x1k", SMALL, {
        int out;
        for (size_t i = 0; i < SMALL; i++)
            llist_get(l, i, &out);
    });

    /* middle insert */
    BENCH("middle insert x1k", SMALL, {
        int val = 99;
        for (int i = 0; i < SMALL; i++)
            llist_insert(l, &val, l->size / 2);
    });

    /* pop front */
    BENCH("pop_front x100k (draining)", (size_t)l->size, {
        int out;
        while (l->size > 0)
            llist_pop_front(l, &out);
    });

    llist_free(l);
}

/* ═══════════════════════════════════════════
   VECTOR VS LINKED LIST HEAD-TO-HEAD
═══════════════════════════════════════════ */

static void bench_head_to_head(void) {
    printf("\n[ Vector vs Linked List — Head-to-Head ]\n");
    printf("  %-55s %12s %12s\n", "Operation", "Vector", "LinkedList");
    printf("  %s\n", "----------------------------------------------------------------------");

    /* push back 100k */
    {
        vector_s *v = NULL; vector_init(&v, sizeof(int));
        linked_list_s *l = NULL; llist_init(&l, sizeof(int));

        struct timespec t = timer_start();
        for (int i = 0; i < LARGE; i++) vector_push(v, &i);
        double vm = timer_end(t);

        t = timer_start();
        for (int i = 0; i < LARGE; i++) llist_push_back(l, &i);
        double lm = timer_end(t);

        printf("  %-55s %11.3fms %11.3fms\n", "push_back x100k", vm, lm);
        vector_free(v); llist_free(l);
    }

    /* sequential get 1k */
    {
        vector_s *v = NULL; vector_init(&v, sizeof(int));
        linked_list_s *l = NULL; llist_init(&l, sizeof(int));
        for (int i = 0; i < SMALL; i++) { vector_push(v, &i); llist_push_back(l, &i); }

        int out;
        struct timespec t = timer_start();
        for (size_t i = 0; i < SMALL; i++) vector_get(v, i, &out);
        double vm = timer_end(t);

        t = timer_start();
        for (size_t i = 0; i < SMALL; i++) llist_get(l, i, &out);
        double lm = timer_end(t);

        printf("  %-55s %11.3fms %11.3fms\n", "sequential get x1k", vm, lm);
        vector_free(v); llist_free(l);
    }

    /* middle insert 1k */
    {
        vector_s *v = NULL; vector_init(&v, sizeof(int));
        linked_list_s *l = NULL; llist_init(&l, sizeof(int));
        for (int i = 0; i < SMALL; i++) { vector_push(v, &i); llist_push_back(l, &i); }

        int val = 99;
        struct timespec t = timer_start();
        for (int i = 0; i < SMALL; i++) vector_insert(v, &val, v->size / 2);
        double vm = timer_end(t);

        t = timer_start();
        for (int i = 0; i < SMALL; i++) llist_insert(l, &val, l->size / 2);
        double lm = timer_end(t);

        printf("  %-55s %11.3fms %11.3fms\n", "middle insert x1k", vm, lm);
        vector_free(v); llist_free(l);
    }

    /* pop front 10k */
    {
        vector_s *v = NULL; vector_init(&v, sizeof(int));
        linked_list_s *l = NULL; llist_init(&l, sizeof(int));
        for (int i = 0; i < MEDIUM; i++) { vector_push(v, &i); llist_push_back(l, &i); }

        int out;
        struct timespec t = timer_start();
        /* vector has no pop_front, simulate with remove(0) */
        while (v->size > 0) vector_remove(v, 0);
        double vm = timer_end(t);

        t = timer_start();
        while (l->size > 0) llist_pop_front(l, &out);
        double lm = timer_end(t);

        printf("  %-55s %11.3fms %11.3fms\n", "pop_front x10k", vm, lm);
        vector_free(v); llist_free(l);
    }
}

/* ═══════════════════════════════════════════
   HASHMAP BENCHMARKS
═══════════════════════════════════════════ */

static void bench_hashmap_at_size(size_t n, size_t bucket_count) {
    hashmap_s *hm = NULL;
    hm_init(&hm, sizeof(int), sizeof(int), bucket_count);

    char label[64];

    /* insert */
    snprintf(label, sizeof(label), "insert x%zu (buckets=%zu)", n, bucket_count);
    BENCH(label, n, {
        for (int i = 0; i < (int)n; i++) {
            int val = i * 2;
            hm_insert(hm, &i, &val);
        }
    });

    /* get */
    snprintf(label, sizeof(label), "get x%zu (buckets=%zu)", n, bucket_count);
    BENCH(label, n, {
        int out;
        for (int i = 0; i < (int)n; i++)
            hm_get(hm, &i, &out);
    });

    /* remove */
    snprintf(label, sizeof(label), "remove x%zu (buckets=%zu)", n, bucket_count);
    BENCH(label, n, {
        for (int i = 0; i < (int)n; i++)
            hm_remove(hm, &i);
    });

    hm_free(hm);
}

static void bench_hashmap(void) {
    printf("\n[ Hashmap ]\n");
    printf("  --- 1k entries ---\n");
    bench_hashmap_at_size(SMALL, 16);

    printf("  --- 10k entries ---\n");
    bench_hashmap_at_size(MEDIUM, 16);

    printf("  --- 100k entries ---\n");
    bench_hashmap_at_size(LARGE, 16);

    printf("\n  --- rehash effect: fixed buckets vs auto-rehash ---\n");
    printf("  %-55s %12s %12s\n", "Operation (100k inserts)", "fixed=128", "fixed=16(rehash)");
    printf("  %s\n", "----------------------------------------------------------------------");
    {
        hashmap_s *hm_large = NULL, *hm_small = NULL;
        hm_init(&hm_large, sizeof(int), sizeof(int), 128);
        hm_init(&hm_small, sizeof(int), sizeof(int), 16);

        struct timespec t = timer_start();
        for (int i = 0; i < LARGE; i++) { int v = i; hm_insert(hm_large, &i, &v); }
        double large_ms = timer_end(t);

        t = timer_start();
        for (int i = 0; i < LARGE; i++) { int v = i; hm_insert(hm_small, &i, &v); }
        double small_ms = timer_end(t);

        printf("  %-55s %11.3fms %11.3fms\n", "insert x100k", large_ms, small_ms);
        printf("  %-55s %11zu   %11zu\n",   "final bucket_count",
               hm_large->bucket_count, hm_small->bucket_count);

        hm_free(hm_large); hm_free(hm_small);
    }
}

/* ═══════════════════════════════════════════
   HASHSET BENCHMARKS
═══════════════════════════════════════════ */

static void bench_hashset_at_size(size_t n, size_t bucket_count) {
    hashset_s *hs = NULL;
    hs_init(&hs, sizeof(int), bucket_count);

    char label[64];

    /* insert */
    snprintf(label, sizeof(label), "insert x%zu (buckets=%zu)", n, bucket_count);
    BENCH(label, n, {
        for (int i = 0; i < (int)n; i++)
            hs_insert(hs, &i);
    });

    /* contains (hits) */
    snprintf(label, sizeof(label), "contains (hit) x%zu (buckets=%zu)", n, bucket_count);
    BENCH(label, n, {
        bool result;
        for (int i = 0; i < (int)n; i++)
            hs_contains(hs, &i, &result);
    });

    /* contains (misses) */
    snprintf(label, sizeof(label), "contains (miss) x%zu (buckets=%zu)", n, bucket_count);
    BENCH(label, n, {
        bool result;
        for (int i = (int)n; i < (int)(n * 2); i++)
            hs_contains(hs, &i, &result);
    });

    /* remove */
    snprintf(label, sizeof(label), "remove x%zu (buckets=%zu)", n, bucket_count);
    BENCH(label, n, {
        for (int i = 0; i < (int)n; i++)
            hs_remove(hs, &i);
    });

    hs_free(hs);
}

static void bench_hashset(void) {
    printf("\n[ Hashset ]\n");
    printf("  --- 1k entries ---\n");
    bench_hashset_at_size(SMALL, 16);

    printf("  --- 10k entries ---\n");
    bench_hashset_at_size(MEDIUM, 16);

    printf("  --- 100k entries ---\n");
    bench_hashset_at_size(LARGE, 16);

    printf("\n  --- duplicate insert stress ---\n");
    {
        hashset_s *hs = NULL;
        hs_init(&hs, sizeof(int), 16);
        int key = 42;
        BENCH("insert same key x100k", LARGE, {
            for (int i = 0; i < LARGE; i++)
                hs_insert(hs, &key);
        });
        printf("  %-55s %11zu\n", "final size (should be 1)", hs->size);
        hs_free(hs);
    }
}

/* ═══════════════════════════════════════════
   MAIN
═══════════════════════════════════════════ */

int main(void) {
    srand(42);  /* fixed seed for reproducibility */

    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║           Data Structure Benchmark Suite             ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");

    bench_vector();
    bench_linked_list();
    bench_head_to_head();
    bench_hashmap();
    bench_hashset();

    printf("\n");
    return 0;
}
