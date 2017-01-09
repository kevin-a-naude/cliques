
#pragma once

#ifndef ARENA_H
#define ARENA_H 1

#include <stddef.h>

#include "new.h"

#define memberptr(objectp,offset)((void*)(((char *)(objectp))+offset))
#define objectptr(memberp,offset)((void*)(((char *)(memberp))-offset))

typedef struct arena_block_t arena_block_t;
struct arena_block_t {
    void* ptr;
    arena_block_t* next;
};

typedef struct {
    size_t unitSize;
    size_t linkOffset;
    size_t unitsPerBlock;
    size_t index;
    arena_block_t* blocks;
    void* freelist;
} arena_t;

static inline void arena(arena_t* self, size_t unitSize, size_t linkOffset)
{
    size_t ptrSize = sizeof(void*);
    size_t unitsPerBlock;
    unitSize = ((unitSize + ptrSize - 1) / ptrSize) * ptrSize;
    unitsPerBlock = (16384 / unitSize);
    if (unitsPerBlock < 1) unitsPerBlock = 1;

    self->unitSize = unitSize;
    self->linkOffset = linkOffset;
    self->unitsPerBlock = unitsPerBlock;
    self->index = unitsPerBlock;
    self->blocks = NULL;
    self->freelist = NULL;
}

void* arena_new_object_out_of_band(arena_t* self);

static inline void* arena_new_object(arena_t* self)
{
    void* object;
    void** memberp;

    if (self->freelist)
    {
        object = self->freelist;
        memberp = (void**)memberptr(object, self->linkOffset);
        self->freelist = *memberp;
        *memberp = NULL;
        return object;
    }
    else
    {
        return arena_new_object_out_of_band(self);
    }
}

static inline void arena_recycle_object(arena_t* self, void* object)
{
    void** lastp = (void**)memberptr(object, self->linkOffset);
    *lastp = self->freelist;
    self->freelist = object;
}

void arena_recycle_linked_list(arena_t* self, void* head);
void arena_destroy(arena_t* self);

#endif
