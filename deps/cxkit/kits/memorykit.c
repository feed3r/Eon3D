/* 
 * Memory utilities kit - utility functions to track memory handling.
 * (C) 2010-2011 Francesco Romani - fromani at gmail dot com. ZLIB licensed.
 */

/** \file memorykit.h
    \brief the Memory handling kit implementation.
*/

#include <unistd.h>

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "memorykit.h"



void *CX__tmalloc(const char *file, int line, size_t size)
{
    void *p = malloc(size);
    if (p == NULL) {
        fprintf(stderr, "[%s:%d] CX__tmalloc(): can't allocate %lu bytes\n",
                        file, line, (unsigned long)size);
    }
    return p;
}

void *CX__tzalloc(const char *file, int line, size_t size)
{
    void *p = calloc(1, size);
    if (p == NULL) {
        fprintf(stderr, "[%s:%d] CX__tzalloc(): can't allocate %lu bytes\n",
                        file, line, (unsigned long)size);
    }
    return p;
}

void *CX__trealloc(const char *file, int line, void *p, size_t size)
{
    p = realloc(p, size);
    if (p == NULL && size > 0) {
        fprintf(stderr, "[%s:%d] CX__trealloc(): can't reallocate to %lu bytes\n",
                        file, line, (unsigned long)size);
    }
    return p;
}

void *CX__tpaalloc(const char *file, int line, size_t size)
{
    long pagesize = sysconf(_SC_PAGESIZE);
    int8_t *base = malloc(size + sizeof(void *) + pagesize);
    int8_t *ptr = NULL;
    unsigned long offset = 0;

    if (base == NULL) {
        fprintf(stderr, "[%s:%d] CX__tpaalloc(): can't allocate %lu bytes\n",
                        file, line, (unsigned long)size);
    } else {
        ptr = base + sizeof(void *);
        offset = (unsigned long)ptr % pagesize;

        if (offset)
            ptr += (pagesize - offset);
        ((void **)ptr)[-1] = base;  /* save the base pointer for freeing */
    }
    return ptr;
}

void CX_pafree(void *ptr)
{
    if (ptr) {
    	free(((void **)ptr)[-1]);
    }
    return;
}

/* vim: et sw=4 ts=4: */

