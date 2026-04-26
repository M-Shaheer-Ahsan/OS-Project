#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include "chunk.h"
#include <stddef.h>

// Struct needed by gui.c to draw the stats
typedef struct {
    int total_chunks;
    int chunks_done;
    size_t total_original_bytes;
    size_t total_compressed_bytes;
    int progress_pct;
    int active_slots[8];
} ProgressStats;

extern ProgressStats g_stats;

int compress_chunks(Chunk *chunks, int count);
void compressor_pause(void);
void compressor_resume(void);
ProgressStats compressor_get_stats(void);

#endif