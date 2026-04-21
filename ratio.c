/*
 * ratio.c fikle Compression ratio reporting utility*/

#include "ratio.h"
#include <stdio.h>
#include <time.h>

RatioReport ratio_compute(size_t original_bytes, size_t compressed_bytes) {
    RatioReport r = {0};
    r.original_kb   = original_bytes   / 1024.0;
    r.compressed_kb = compressed_bytes / 1024.0;

    if (original_bytes > 0) {
        r.ratio         = 1.0 - ((double)compressed_bytes / (double)original_bytes);
        r.percent_saved = r.ratio * 100.0;
    }
    return r;
}

void ratio_print(const RatioReport *r) {
    printf("---- Compression Report ----\n");
    printf("Original  : %.2f KB\n", r->original_kb);
    printf("Compressed: %.2f KB\n", r->compressed_kb);
    printf("Saved     : %.1f%%\n",  r->percent_saved);
    printf("----------------------------\n");
}

void ratio_log_to_file(const RatioReport *r, const char *path) {
    FILE *f = fopen(path, "a");
    if (!f) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(f, "[%04d-%02d-%02d %02d:%02d:%02d] "
               "Original: %.2f KB | Compressed: %.2f KB | Saved: %.1f%%\n",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec,
            r->original_kb, r->compressed_kb, r->percent_saved);

    fclose(f);
}