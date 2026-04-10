#include <stdio.h>
#include <stdlib.h>
#include "writer.h"

int write_output(const char *filename, Chunk *chunks, int chunk_count, size_t original_size) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Error: could not open output file '%s'\n", filename);
        return -1;
    }

    fwrite(&chunk_count, sizeof(int), 1, file);
    fwrite(&original_size, sizeof(size_t), 1, file);

    for (int i = 0; i < chunk_count; i++) {
        fwrite(&chunks[i].id, sizeof(int), 1, file);
        fwrite(&chunks[i].original_size, sizeof(size_t), 1, file);
        fwrite(&chunks[i].compressed_size, sizeof(size_t), 1, file);
        fwrite(chunks[i].data, 1, chunks[i].compressed_size, file);
    }

    fclose(file);
    printf("Output written to '%s'\n", filename);
    return 0;
}