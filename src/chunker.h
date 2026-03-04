#ifndef CHUNKER_H
#define CHUNKER_H

#include "chunk.h"

int chunk_file(const char *filename, Chunk **chunks, int *chunk_count);
void free_chunks(Chunk *chunks, int chunk_count);

#endif