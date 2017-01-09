
#pragma once

#ifndef NEW_H
#define NEW_H 1

#include <stddef.h>
#include <stdlib.h>

#include "fatal.h"

static inline void* new_object(size_t size)
{
    void* p = calloc(1, size);
    if (p == NULL) fatal_error("out of memory");
    return p;
}

static inline void* new_array(size_t element_size, size_t count)
{
    return new_object(element_size * count);
}

static inline void* free_object(void* obj)
{
    if (obj != NULL) free(obj);
    return NULL;
}

#endif
