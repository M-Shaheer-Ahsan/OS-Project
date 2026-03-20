#include "compressor.h"
#include "sync.h"
#include "chunk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <zlib.h>

/* ─── Global Progress Stats ─────────────────────────────────────────── */

ProgressStats g_stats = {0};

/* ─── Priority Queue (min-heap by chunk->priority) ──────────────────── */

static Chunk         **pq_heap = NULL;
static int             pq_size = 0;
static int             pq_cap  = 0;

static void pq_swap(int i, int j) {
    Chunk *tmp  = pq_heap[i];
    pq_heap[i]  = pq_heap[j];
    pq_heap[j]  = tmp;
}

static void pq_bubble_up(int i) {
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (pq_heap[parent]->priority <= pq_heap[i]->priority) break;
        pq_swap(i, parent);
        i = parent;
    }
}

static void pq_bubble_down(int i) {
    while (1) {
        int smallest = i;
        int left  = 2 * i + 1;
        int right = 2 * i + 2;
        if (left  < pq_size && pq_heap[left]->priority  < pq_heap[smallest]->priority) smallest = left;
        if (right < pq_size && pq_heap[right]->priority < pq_heap[smallest]->priority) smallest = right;
        if (smallest == i) break;
        pq_swap(i, smallest);
        i = smallest;
    }
}

static int pq_push(Chunk *c) {
    if (pq_size == pq_cap) {
        int new_cap  = pq_cap ? pq_cap * 2 : 8;
        Chunk **tmp  = realloc(pq_heap, new_cap * sizeof(Chunk *));
        if (!tmp) return -1;
        pq_heap = tmp;
        pq_cap  = new_cap;
    }
    pq_heap[pq_size++] = c;
    pq_bubble_up(pq_size - 1);
    return 0;
}

static Chunk *pq_pop(void) {
    if (pq_size == 0) return NULL;
    Chunk *top  = pq_heap[0];
    pq_heap[0]  = pq_heap[--pq_size];
    pq_bubble_down(0);
    return top;
}

static void pq_free(void) {
    free(pq_heap);
    pq_heap = NULL;
    pq_size = pq_cap = 0;
}