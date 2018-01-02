#include <stdio.h>
#include <stdlib.h>

void* safe_malloc(size_t n, char* file, unsigned long line) {
    void* p = malloc(n);
    if (!p) {
        fprintf(stderr, "[%s:%zu]Out of memory(%zu bytes)\n", file, line, n);
        exit(EXIT_FAILURE);
    }
    return p;
}
