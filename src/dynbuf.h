#ifndef DYNBUF_H
#define DYNBUF_H

typedef struct {
    char **lines;     // Array of string pointers
    int    count;     // How many lines are stored
    int    capacity;  // How much space is currently allocated
} DynBuf;

void dynbuf_init(DynBuf *buf, int initial_capacity);
void dynbuf_append(DynBuf *buf, const char *line);
void dynbuf_free(DynBuf *buf);

#endif