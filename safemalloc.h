#ifndef SAFEMALLOC_H
#define SAFEMALLOC_H

void* safe_malloc(size_t n, char* file, unsigned long line);
#define SAFEMALLOC(n) safe_malloc(n, __FILE__, __LINE__)

#endif
