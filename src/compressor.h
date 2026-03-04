#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include "chunk.h"

void compress_chunks(Chunk *chunks, int chunk_count, int max_threads);

#endif