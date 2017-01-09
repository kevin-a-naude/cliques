
#pragma once

#ifndef INTSET_H
#define INTSET_H 1

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "new.h"
#include "arena.h"
#include "bitmanip.h"

typedef struct {
    arena_t setArena;
    arena_t nodeArena;
    size_t domain;
} setmanager_t;

typedef struct intset_node_t intset_node_t;
struct intset_node_t {
    intset_node_t* next;
    uint64_t bits;
    size_t index;
};

typedef struct {
    intset_node_t* head;
    size_t domain;

    setmanager_t* manager;
} intset_t;

typedef struct {
    intset_t bv;
    uint64_t currentMask;
} intset_walk_t;


/* setmanager_t */

static inline void setmanager(setmanager_t* self, size_t domain)
{
    self->domain = domain;
    arena(&self->setArena, sizeof(intset_t), offsetof(intset_t, head));
    arena(&self->nodeArena, sizeof(intset_node_t), offsetof(intset_node_t, next));
}

static inline void setmanager_destroy(setmanager_t* self)
{
    arena_destroy(&self->nodeArena);
    arena_destroy(&self->setArena);
}

static inline void setmanager_repurpose(setmanager_t* self, size_t domain)
{
    setmanager_destroy(self);
    setmanager(self, domain);
}

/* intset_walk_t */

static inline void intset_walk(intset_walk_t* self, intset_t* bv)
{
    self->bv = *bv;

    self->currentMask = UINT64_C(0xFFFFFFFFFFFFFFFF);
}

static inline bool intset_walk_next(intset_walk_t* self, size_t* bitIndex)
{
    bool result = false;
    intset_node_t* node = self->bv.head;
    if (node)
    {
        size_t currentIndex = node->index;
        uint64_t currentMask = self->currentMask;
        uint64_t currentBits = node->bits & currentMask;

        if (currentBits == 0)
        {
            while ((currentBits == 0) && (node = node->next))
            {
                currentBits = node->bits;
                currentMask = UINT64_C(0xFFFFFFFFFFFFFFFF);
                currentIndex = node->index;
            }
            self->bv.head = node;
        }

        if (currentBits != 0)
        {
            uint64_t bit = lowest_bit(currentBits);
            currentMask = all_higher_bits(bit);
            *bitIndex = currentIndex + bit_to_index(bit);
            result = true;
        }

        self->currentMask = currentMask;
    }
    return result;
}

/* intset_t */

static inline void intset_fill(intset_t* self)
{
    setmanager_t* manager = self->manager;

    size_t i;
    size_t domain = self->domain;

    if (domain > 0)
    {
        size_t highIndex = (domain - 1) & ~(size_t)63;
        uint64_t highMask = ~all_higher_bits(single_bit((domain + 63) & 63));

        intset_node_t** curp = &self->head;
        intset_node_t* cur = self->head;

        for (i = 0; i < highIndex; i += 64)
        {
            if ((cur == NULL) || (i < cur->index))
            {
                intset_node_t* node = (intset_node_t*)arena_new_object(&manager->nodeArena);
                node->next = cur;
                node->bits = UINT64_C(0xFFFFFFFFFFFFFFFF);
                node->index = i;
                *curp = node;
                curp = &node->next;
            }
            else
            {
                cur->bits = UINT64_C(0xFFFFFFFFFFFFFFFF);
                curp = &cur->next;
                cur = cur->next;
            }
        }
        if (cur == NULL)
        {
            intset_node_t* node = (intset_node_t*)arena_new_object(&manager->nodeArena);
            node->next = cur;
            node->bits = highMask;
            node->index = highIndex;
            *curp = node;
        }
        else
        {
            cur->bits = highMask;
        }
    }
}

static inline void intset_clear(intset_t* self, bool trim)
{
    setmanager_t* manager = self->manager;

    if (trim)
    {
        arena_recycle_linked_list(&manager->nodeArena, self->head);
        self->head = NULL;
    }
    else
    {
        intset_node_t* cur = self->head;
        while (cur)
        {
            cur->bits = 0;
            cur = cur->next;
        }
    }
}

static inline intset_t* intset(size_t domain, bool value, setmanager_t* manager)
{
    intset_t* self = (intset_t*) arena_new_object(&manager->setArena);
    self->head = NULL;
    self->domain = domain;
    self->manager = manager;
    if (value)
        intset_fill(self);
    return self;
}

static inline void intset_non(intset_t* self)
{
    self->head = NULL;
    self->manager = NULL;
    self->domain = 0;
}

static inline void intset_recycle(intset_t* self)
{
    setmanager_t* manager = self->manager;
    if (manager != NULL)
    {
        arena_recycle_linked_list(&manager->nodeArena, self->head);
        self->head = NULL;
        self->domain = 0;
    }
    arena_recycle_object(&manager->setArena, self);
}

static inline intset_t* intset_copy(const intset_t* pattern, setmanager_t* manager)
{
    intset_t* self = (intset_t*) arena_new_object(&manager->setArena);

    intset_node_t** curp = &self->head;
    intset_node_t* cur = pattern->head;

    self->manager = manager;
    self->head = NULL;
    self->domain = pattern->domain;

    while (cur)
    {
        intset_node_t* node = (intset_node_t*) arena_new_object(&manager->nodeArena);
        node->next = NULL;
        node->bits = cur->bits;
        node->index = cur->index;
        *curp = node;
        curp = &node->next;

        cur = cur->next;
    }

    return self;
}

static inline intset_t* intset_copy_remove(const intset_t* patternA, const intset_t* patternB)
{
    setmanager_t* manager = patternA->manager;
    intset_t* self = (intset_t*) arena_new_object(&manager->setArena);

    intset_node_t** curp = &self->head;

    intset_node_t* a = patternA->head;
    intset_node_t* b = patternB->head;

    self->manager = manager;
    self->head = NULL;
    self->domain = patternA->domain;

    while ((a != NULL) && (b != NULL))
    {
        uint64_t bits;

        while ((a != NULL) && (a->index < b->index))
        {
            intset_node_t* node = (intset_node_t*) arena_new_object(&manager->nodeArena);
            node->next = NULL;
            node->bits = a->bits;
            node->index = a->index;
            *curp = node;
            curp = &node->next;

            a = a->next;
            if (a == NULL) goto stop;
        }

        while ((b != NULL) && (b->index < a->index))
        {
            b = b->next;
            if (b == NULL) goto stop;
        }

        if (a->index != b->index) continue;

        bits = a->bits & ~b->bits;
        if (bits)
        {
            intset_node_t* node = (intset_node_t*) arena_new_object(&manager->nodeArena);
            node->next = NULL;
            node->bits = bits;
            node->index = a->index;
            *curp = node;
            curp = &node->next;
        }

        a = a->next;
        b = b->next;
    }

    while (a != NULL)
    {
        intset_node_t* node = (intset_node_t*) arena_new_object(&manager->nodeArena);
        node->next = NULL;
        node->bits = a->bits;
        node->index = a->index;
        *curp = node;
        curp = &node->next;

        a = a->next;
    }
stop:
    return self;
}

static inline intset_t* intset_copy_intersect(const intset_t* patternA, const intset_t* patternB)
{
    setmanager_t* manager = patternA->manager;
    intset_t* self = (intset_t*) arena_new_object(&manager->setArena);

    intset_node_t** curp = &self->head;

    intset_node_t* a = patternA->head;
    intset_node_t* b = patternB->head;

    self->manager = manager;
    self->head = NULL;
    self->domain = patternA->domain;

    while ((a != NULL) && (b != NULL))
    {
        uint64_t bits;

        while ((a != NULL) && (a->index < b->index))
        {
            a = a->next;
            if (a == NULL) goto stop;
        }


        while ((b != NULL) && (b->index < a->index))
        {
            b = b->next;
            if (b == NULL) goto stop;
        }

        if (a->index != b->index) continue;

        bits = a->bits & b->bits;
        if (bits)
        {
            intset_node_t* node = (intset_node_t*) arena_new_object(&manager->nodeArena);
            node->next = NULL;
            node->bits = bits;
            node->index = a->index;
            *curp = node;
            curp = &node->next;
        }
        a = a->next;
        b = b->next;
    }
stop:
    return self;
}

static inline bool intset_is_empty(const intset_t* self)
{
    intset_node_t* cur = self->head;
    if ((cur != NULL) && (cur->bits != 0))
        return false;
    else
    {
        while ((cur != NULL) && (cur->bits == 0)) cur = cur->next;
        return (cur == NULL);
    }
}

#define intset_find(thehead,theindex,thecurp,thecur)\
    intset_node_t** thecurp = &thehead;\
    intset_node_t* thecur = thehead;\
    while ((thecur != NULL) && (thecur->index < theindex))\
    {\
        thecurp = &thecur->next;\
        thecur = thecur->next;\
    }

#define intset_find_const(thehead,theindex,thecurp,thecur)\
    intset_node_t* const * thecurp = &thehead;\
    const intset_node_t* thecur = thehead;\
    while ((thecur != NULL) && (thecur->index < theindex))\
    {\
        thecurp = &thecur->next;\
        thecur = thecur->next;\
    }

static inline void intset_add(intset_t* self, size_t value)
{
    setmanager_t* manager = self->manager;

    size_t pos = value & 63;
    size_t index = value & ~(size_t)63;
    uint64_t bit = single_bit(pos);

    intset_find(self->head, index, curp, cur)

    if ((cur != NULL) && (cur->index == index))
    {
        cur->bits |= bit;
    }
    else
    {
        intset_node_t* node = (intset_node_t*) arena_new_object(&manager->nodeArena);
        node->next = cur;
        node->bits = bit;
        node->index = index;
        *curp = node;
    }
}

static inline void intset_remove(intset_t* self, size_t value, bool trim)
{
    setmanager_t* manager = self->manager;

    size_t pos = value & 63;
    size_t index = value & ~(size_t)63;
    uint64_t bit = single_bit(pos);

    intset_find(self->head, index, curp, cur)

    if ((cur != NULL) && (cur->index == index))
    {
        uint64_t bits = cur->bits & ~bit;
        if (bits || !trim)
        {
            cur->bits = bits;
        }
        else
        {
            intset_node_t* node = cur;
            *curp = cur->next;
            arena_recycle_object(&manager->nodeArena, node);
        }
    }
}

static inline bool intset_contains(const intset_t* self, size_t value)
{
    size_t pos = value & 63;
    size_t index = value & ~(size_t)63;
    uint64_t bit = single_bit(pos);

    intset_find_const(self->head, index, curp, cur)

    if ((cur != NULL) && (cur->index == index))
    {
        return (cur->bits & bit) != 0;
    }
    return false;
}

static inline bool intset_get(const intset_t* self, size_t value)
{
    return intset_contains(self, value);
}

static inline void intset_set(intset_t* self, size_t value, bool enabled, bool trim)
{
    if (enabled)
        intset_add(self, value);
    else
        intset_remove(self, value, trim);
}

static inline bool intset_first(const intset_t* self, size_t* value)
{
    intset_node_t* cur = self->head;
    while ((cur != NULL) && (cur->bits == 0)) cur = cur->next;
    if (cur != NULL)
    {
        uint64_t bits = cur->bits;
        *value = cur->index + lowest_bit_index(bits);
        return true;
    }
    else
        return false;
}

static inline void intset_complement(intset_t* self, bool trim)
{
    setmanager_t* manager = self->manager;

    size_t i;
    size_t domain = self->domain;

    if (domain > 0)
    {
        size_t highIndex = (domain - 1) & ~(size_t)63;
        uint64_t highMask = ~all_higher_bits(single_bit((self->domain + 63) & 63));

        intset_node_t** curp = &self->head;
        intset_node_t* cur = self->head;

        for (i = 0; i < highIndex; i += 64)
        {
            if ((cur == NULL) || (i < cur->index))
            {
                intset_node_t* node = (intset_node_t*) arena_new_object(&manager->nodeArena);
                node->next = cur;
                node->bits = UINT64_C(0xFFFFFFFFFFFFFFFF);
                node->index = i;
                *curp = node;
                curp = &node->next;
            }
            else
            {
                uint64_t bits = (~cur->bits) & UINT64_C(0xFFFFFFFFFFFFFFFF);
                if (bits || !trim)
                {
                    cur->bits = bits;
                    curp = &cur->next;
                    cur = cur->next;
                }
                else
                {
                    intset_node_t* node = cur;
                    cur = cur->next;
                    *curp = cur;
                    arena_recycle_object(&manager->nodeArena, node);
                }
            }
        }
        if (cur == NULL)
        {
            intset_node_t* node = (intset_node_t*) arena_new_object(&manager->nodeArena);
            node->next = cur;
            node->bits = highMask;
            node->index = highIndex;
            *curp = node;
        }
        else
        {
            cur->bits = (~cur->bits) & highMask;
        }
    }
}

static inline void intset_intersect(intset_t* self, const intset_t* pattern, bool trim)
{
    setmanager_t* manager = self->manager;

    intset_node_t** curp = &self->head;
    intset_node_t* cur = self->head;
    intset_node_t* other = pattern->head;

    while ((cur != NULL) && (other != NULL))
    {
        uint64_t bits;

        while ((cur != NULL) && (cur->index < other->index))
        {
            if (!trim)
            {
                cur->bits = 0;
                curp = &cur->next;
                cur = cur->next;
            }
            else
            {
                intset_node_t* node = cur;
                cur = cur->next;
                *curp = cur;
                arena_recycle_object(&manager->nodeArena, node);
            }
        }

        if (cur == NULL) break;

        while ((other != NULL) && (other->index < cur->index))
        {
            other = other->next;
        }

        if (other == NULL) break;

        if (cur->index != other->index) continue;

        bits = cur->bits & other->bits;
        if (bits || !trim)
        {
            cur->bits = bits;
            curp = &cur->next;
            cur = cur->next;
        }
        else
        {
            intset_node_t* node = cur;
            cur = cur->next;
            *curp = cur;
            arena_recycle_object(&manager->nodeArena, node);
        }
        other = other->next;
    }

    while (cur != NULL)
    {
        if (!trim)
        {
            cur->bits = 0;
            curp = &cur->next;
            cur = cur->next;
        }
        else
        {
            intset_node_t* node = cur;
            cur = cur->next;
            *curp = cur;
            arena_recycle_object(&manager->nodeArena, node);
        }
    }
}

static inline void intset_union(intset_t* self, const intset_t* pattern)
{
    setmanager_t* manager = self->manager;

    intset_node_t** curp = &self->head;
    intset_node_t* cur = self->head;
    intset_node_t* other = pattern->head;

    while ((cur != NULL) && (other != NULL))
    {
        uint64_t bits;

        while ((cur != NULL) && (cur->index < other->index))
        {
            curp = &cur->next;
            cur = cur->next;
        }

        if (cur == NULL) break;

        while ((other != NULL) && (other->index < cur->index))
        {
            if (other->bits)
            {
                intset_node_t* node = (intset_node_t*) arena_new_object(&manager->nodeArena);
                node->next = cur;
                node->bits = other->bits;
                node->index = other->index;
                *curp = node;
                curp = &node->next;
            }

            other = other->next;
        }

        if (other == NULL) break;

        if (cur->index != other->index) continue;

        bits = cur->bits | other->bits;
        cur->bits = bits;
        cur = cur->next;
        other = other->next;
    }

    while (other != NULL)
    {
        if (other->bits)
        {
            intset_node_t* node = (intset_node_t*) arena_new_object(&manager->nodeArena);
            node->next = cur;
            node->bits = other->bits;
            node->index = other->index;
            *curp = node;
            curp = &node->next;
        }

        other = other->next;
    }
}

static inline void intset_remove_all(intset_t* self, const intset_t* pattern, bool trim)
{
    setmanager_t* manager = self->manager;

    intset_node_t** curp = &self->head;
    intset_node_t* cur = self->head;
    intset_node_t* other = pattern->head;

    while ((cur != NULL) && (other != NULL))
    {
        uint64_t bits;

        while ((cur != NULL) && (cur->index < other->index))
        {
            curp = &cur->next;
            cur = cur->next;
        }

        if (cur == NULL) break;

        while ((other != NULL) && (other->index < cur->index))
        {
            other = other->next;
        }

        if (other == NULL) break;

        if (cur->index != other->index) continue;

        bits = cur->bits & ~other->bits;
        if (bits || !trim)
        {
            cur->bits = bits;
            curp = &cur->next;
            cur = cur->next;
        }
        else
        {
            intset_node_t* node = cur;
            cur = cur->next;
            *curp = cur;
            arena_recycle_object(&manager->nodeArena, node);
        }
        other = other->next;
    }
}

static inline size_t intset_count(const intset_t* self)
{
    intset_node_t* cur = self->head;
    size_t sum = 0;
    while (cur != NULL)
    {
        uint64_t bits = cur->bits;
        sum += count_bits(bits);
        cur = cur->next;
    }
    return sum;
}

static inline size_t intset_count_until(const intset_t* self, size_t limit, size_t* w)
{
    intset_node_t* cur = self->head;
    size_t sum = 0;
    size_t theW = self->domain;
    while (cur != NULL)
    {
        uint64_t bits = cur->bits;
        if (bits)
        {
            sum += count_bits(bits);
            if (bits) theW = cur->index + bit_to_index(lowest_bit(bits)); /// any bit will do
            if (sum >= limit) break;
        }
        cur = cur->next;
    }
    *w = theW;
    return sum;
}

static inline size_t intset_count_common(const intset_t* self, const intset_t* pattern)
{
    size_t sum = 0;
    size_t theW = self->domain;

    intset_node_t* cur = self->head;
    intset_node_t* other = pattern->head;

    while ((cur != NULL) && (other != NULL))
    {
        uint64_t bits;

        while ((cur != NULL) && (cur->index < other->index))
        {
            cur = cur->next;
        }

        if (cur == NULL) break;

        while ((other != NULL) && (other->index < cur->index))
        {
            other = other->next;
        }

        if (other == NULL) break;

        if (cur->index != other->index) continue;

        bits = cur->bits & other->bits;
        sum += count_bits(bits);

        cur = cur->next;
        other = other->next;
    }
    return sum;
}

static inline size_t intset_count_common_until(const intset_t* self, const intset_t* pattern, size_t limit, size_t* w)
{
    size_t sum = 0;
    size_t theW = self->domain;

    intset_node_t* cur = self->head;
    intset_node_t* other = pattern->head;

    while ((cur != NULL) && (other != NULL))
    {
        uint64_t bits;

        while ((cur != NULL) && (cur->index < other->index))
        {
            cur = cur->next;
        }

        if (cur == NULL) break;

        while ((other != NULL) && (other->index < cur->index))
        {
            other = other->next;
        }

        if (other == NULL) break;

        if (cur->index != other->index) continue;

        bits = cur->bits & other->bits;
        if (bits)
        {
            theW = cur->index + bit_to_index(lowest_bit(bits)); /// any bit will do
            sum += count_bits(bits);
            if (sum >= limit) break;
        }

        cur = cur->next;
        other = other->next;
    }
    *w = theW;
    return sum;
}

static inline size_t intset_count_common_inverse_until(const intset_t* self, const intset_t* pattern, size_t limit, size_t* w)
{
    size_t sum = 0;
    size_t theW = self->domain;

    intset_node_t* cur = self->head;
    intset_node_t* other = pattern->head;

    while ((cur != NULL) && (other != NULL))
    {
        uint64_t bits;

        while ((cur != NULL) && (cur->index < other->index))
        {
            if (cur->bits)
            {
                theW = cur->index + bit_to_index(lowest_bit(cur->bits)); /// any bit will do
                sum += count_bits(cur->bits);
                if (sum >= limit) goto stop;
            }
            cur = cur->next;
            if (cur == NULL) goto stop;
        }


        while ((other != NULL) && (other->index < cur->index))
        {
            other = other->next;
            if (other == NULL) goto stop;
        }

        if (cur->index != other->index) continue;

        bits = cur->bits & ~other->bits;
        if (bits)
        {
            theW = cur->index + bit_to_index(lowest_bit(bits)); /// any bit will do
            sum += count_bits(bits);
            if (sum >= limit) break;
        }

        cur = cur->next;
        other = other->next;
    }
stop:
    *w = theW;
    return sum;
}

static inline void intset_swap_entries(intset_t* self, size_t i, size_t j, bool trim)
{
    bool temp = intset_get(self, i);
    intset_set(self, i, intset_get(self, j), trim);
    intset_set(self, j, temp, trim);
}

#undef intset_find_const
#undef intset_find

#endif
