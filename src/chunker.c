#include <stdio.h>
#include <stdlib.h>
#include "chunker.h"

int chunk_file(const char *filename, Chunk **chunks, int *chunk_count) {
    // TODO: Shaheer — open file, split into 1MB chunks, populate chunks array
    return 0;
}

void free_chunks(Chunk *chunks, int chunk_count) {
    // TODO: Shaheer — free all chunk data buffers
}