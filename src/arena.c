
#include "arena.h"

void* arena_new_object_out_of_band(arena_t* self)
{
    void* object;
    void** memberp;

    size_t index = self->index;
    if (index >= self->unitsPerBlock)
    {
        arena_block_t* block = new_object(sizeof(arena_block_t));
        block->next = self->blocks;
        block->ptr = new_object(self->unitsPerBlock * self->unitSize);
        self->blocks = block;
        index = 0;
    }
    object = memberptr(self->blocks->ptr, self->unitSize * index);
    memberp = (void**)memberptr(object, self->linkOffset);

    self->index = index + 1;
    *memberp = NULL;
    return object;
}

void arena_recycle_linked_list(arena_t* self, void* head)
{
    void* recent = self->freelist;
    while (head)
    {
        void** nextp = (void**)memberptr(head, self->linkOffset);
        void* next = *nextp;
        *nextp = recent;
        recent = head;
        head = next;
    }
    self->freelist = recent;
}

void arena_destroy(arena_t* self)
{
    arena_block_t* next = self->blocks;
    while (next != NULL)
    {
        arena_block_t* cur = next;
        next = next->next;

        free_object(cur->ptr);
        free_object(cur);
    }
}
