#ifndef _PR_H
#define _PR_H

#include <unistd.h>
#include <stdlib.h>

#ifdef _PR_PRINT
#include <stdio.h>
#endif

#include "graph.h"
#include "thr_pool.h"

/* I could only submit the header file, this should typically come before the include */
#ifndef _PR_SAFE_ALLOC
#define _PR_SAFE_ALLOC 1
#endif

#ifndef PR_MALLOC
#ifdef _PR_SAFE_ALLOC
#define PR_MALLOC(size) nn_malloc_debug((size), __FILE__, __LINE__)
#else
#define PR_MALLOC malloc
#endif
#endif

#ifdef _PR_SAFE_ALLOC
static inline void* nn_malloc_debug(size_t size, const char* file, int line)
{
    void* ptr = malloc(size);
    if (ptr == NULL)
    {
#ifdef _PR_PRINT
        fprintf(stderr, "Error: failed to allocate memory in %s, %d\n", file, line);
#endif
        exit(EXIT_FAILURE);
    }
    return ptr;
}
#endif

#ifdef _PR_SAFE_ALLOC
void* pr_malloc_debug(size_t size, const char* file, int line);
#endif

void PageRank(Graph *graph, int n, float* ranks);

#endif /* _PR_H */
