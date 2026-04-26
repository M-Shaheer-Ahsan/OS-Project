#include "dynbuf.h"
#include <stdlib.h>
#include <string.h>

void dynbuf_init(DynBuf *buf, int initial_capacity) {
    buf->capacity = initial_capacity > 0 ? initial_capacity : 64;
    buf->count = 0;
    buf->lines = malloc(buf->capacity * sizeof(char*));
}

void dynbuf_append(DynBuf *buf, const char *line) {
    if (buf->count >= buf->capacity) {
        buf->capacity = (buf->capacity == 0) ? 64 : buf->capacity * 2;
        buf->lines = realloc(buf->lines, buf->capacity * sizeof(char*));
    }
    buf->lines[buf->count] = strdup(line);
    buf->count++;
}

void dynbuf_free(DynBuf *buf) {
    if (!buf->lines) return;
    for (int i = 0; i < buf->count; i++) free(buf->lines[i]);
    free(buf->lines);
    buf->lines = NULL;
    buf->count = buf->capacity = 0;
}