#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include "decompressor.h"

int decompress_file(const char *input, const char *output) {
    FILE *in = fopen(input, "rb");
    if (!in) {
        fprintf(stderr, "Error: could not open compressed file '%s'\n", input);
        return -1;
    }

    FILE *out = fopen(output, "wb");
    if (!out) {
        fprintf(stderr, "Error: could not open output file '%s'\n", output);
        fclose(in);
        return -1;
    }
    return 0;
}