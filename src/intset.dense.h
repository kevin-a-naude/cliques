
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
    arena_t arrayArena;
    size_t domain;
} setmanager_t;

typedef struct {
    size_t domain;
    size_t arraySize;
    uint64_t* array;

    setmanager_t* manager;
} intset_t;

typedef struct {
    intset_t bv;
    uint64_t currentMask;
    size_t currentIndex;
} intset_walk_t;


/* setmanager_t */

static inline void setmanager(setmanager_t* self, size_t domain)
{
    size_t arraySize = ((domain + 63) / 64) == 0? 1 : ((domain + 63) / 64);
    self->domain = domain;
    arena(&self->setArena, sizeof(intset_t), 0);
    arena(&self->arrayArena, sizeof(uint64_t) * arraySize, 0);
}

static inline void setmanager_destroy(setmanager_t* self)
{
    arena_destroy(&self->arrayArena);
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
    self->currentIndex = 0;
}

static inline bool intset_walk_next(intset_walk_t* self, size_t* bitIndex)
{
    bool result = false;
    size_t currentIndex = self->currentIndex;
    size_t arrayIndex = currentIndex / 64;
    if (arrayIndex < self->bv.arraySize)
    {
        uint64_t currentMask = self->currentMask;
        uint64_t currentBits = self->bv.array[arrayIndex] & currentMask;

        while ((currentBits == 0) && (arrayIndex + 1 < self->bv.arraySize))
        {
            arrayIndex++;
            currentBits = self->bv.array[arrayIndex];
            currentMask = UINT64_C(0xFFFFFFFFFFFFFFFF);
            currentIndex += 64;
        }

        if (currentBits != 0)
        {
            uint64_t bit = lowest_bit(currentBits);
            currentBits ^= bit;
            currentMask = all_higher_bits(bit);
            *bitIndex = currentIndex + bit_to_index(bit);
            result = true;
        }

        self->currentIndex = currentIndex;
        self->currentMask = currentMask;
    }
    return result;
}

/* intset_t */

static inline void intset_fill(intset_t* self)
{
    size_t i;
    size_t domain = self->domain;

    if (domain > 0)
    {
        size_t arraySize = self->arraySize;
        uint64_t highMask = ~all_higher_bits(single_bit((domain + 63) & 63));

        for (i = 0; i < arraySize - 1; i++)
        {
            self->array[i] = UINT64_C(0xFFFFFFFFFFFFFFFF);
        }
        self->array[arraySize - 1] = highMask;
    }
}

static inline void intset_clear(intset_t* self, bool trim)
{
    size_t i, arraySize = self->arraySize;
    for (i = 0; i < arraySize; i++)
    {
        self->array[i] = 0;
    }
}

static inline intset_t* intset(size_t domain, bool value, setmanager_t* manager)
{
    intset_t* self = (intset_t*) arena_new_object(&manager->setArena);
    self->domain = domain;
    self->manager = manager;
    self->arraySize = ((domain + 63) / 64) == 0? 1 : ((domain + 63) / 64);
    self->array = (uint64_t*) arena_new_object(&manager->arrayArena);

    if (value)
        intset_fill(self);
    else
        intset_clear(self, true);
    return self;
}

static inline void intset_non(intset_t* self)
{
    self->manager = NULL;
    self->domain = 0;
    self->arraySize = 0;
    self->array = NULL;
}

static inline void intset_recycle(intset_t* self)
{
    setmanager_t* manager = self->manager;
    if (manager != NULL)
    {
        if (self->array) arena_recycle_object(&manager->arrayArena, self->array);
        self->array = NULL;
        self->domain = 0;
    }
    arena_recycle_object(&manager->setArena, self);
}

static inline intset_t* intset_copy(const intset_t* pattern, setmanager_t* manager)
{
    size_t i, domain = manager->domain, arraySize = ((domain + 63) / 64) == 0? 1 : ((domain + 63) / 64);
    intset_t* self = (intset_t*) arena_new_object(&manager->setArena);
    self->domain = domain;
    self->manager = manager;
    self->arraySize = arraySize;
    self->array = (uint64_t*) arena_new_object(&manager->arrayArena);

    for (i = 0; i < arraySize; i++)
    {
        self->array[i] = pattern->array[i];
    }

    return self;
}

static inline intset_t* intset_copy_remove(const intset_t* patternA, const intset_t* patternB)
{
    setmanager_t* manager = patternA->manager;
    size_t i, domain = manager->domain, arraySize = ((domain + 63) / 64) == 0? 1 : ((domain + 63) / 64);
    intset_t* self = (intset_t*) arena_new_object(&manager->setArena);
    self->domain = domain;
    self->manager = manager;
    self->arraySize = arraySize;
    self->array = (uint64_t*) arena_new_object(&manager->arrayArena);

    for (i = 0; i < arraySize; i++)
    {
        self->array[i] = patternA->array[i] & ~patternB->array[i];
    }

    return self;
}

static inline intset_t* intset_copy_intersect(const intset_t* patternA, const intset_t* patternB)
{
    setmanager_t* manager = patternA->manager;
    size_t i, domain = manager->domain, arraySize = ((domain + 63) / 64) == 0? 1 : ((domain + 63) / 64);
    intset_t* self = (intset_t*) arena_new_object(&manager->setArena);
    self->domain = domain;
    self->manager = manager;
    self->arraySize = arraySize;
    self->array = (uint64_t*) arena_new_object(&manager->arrayArena);

    for (i = 0; i < arraySize; i++)
    {
        self->array[i] = patternA->array[i] & patternB->array[i];
    }

    return self;
}

static inline bool intset_is_empty(const intset_t* self)
{
    if (self->array[0] != 0)
        return false;
    else
    {
        size_t i, arraySize = self->arraySize;
        for (i = 0; i < arraySize; i++)
        {
            if (self->array[i] != 0) return false;
        }
        return true;
    }
}

static inline void intset_add(intset_t* self, size_t value)
{
    size_t index = value / 64;
    uint64_t bit = single_bit(value % 64);
    self->array[index] |= bit;
}

static inline void intset_remove(intset_t* self, size_t value, bool trim)
{
    size_t index = value / 64;
    uint64_t bit = single_bit(value % 64);
    self->array[index] &= ~bit;
}

static inline bool intset_contains(const intset_t* self, size_t value)
{
    size_t pos = value & 63;
    size_t index = value / 64;
    uint64_t bit = single_bit(pos);
    return (self->array[index] & bit) != 0;
}

static inline bool intset_get(const intset_t* self, size_t value)
{
    return intset_contains(self, value);
}

static inline void intset_set(intset_t* self, size_t value, bool enabled, bool trim)
{
    size_t pos = value & 63;
    size_t index = value / 64;
    uint64_t bit = single_bit(pos);
    if (enabled)
        self->array[index] |= bit;
    else
        self->array[index] &= ~bit;
}

static inline bool intset_first(const intset_t* self, size_t* value)
{
    size_t arrayIndex = 0;
    size_t currentIndex = 0;
    size_t arraySize = self->arraySize;
    uint64_t currentBits = self->array[arrayIndex];
    while ((currentBits == 0) && (arrayIndex + 1 < arraySize))
    {
        arrayIndex++;
        currentBits = self->array[arrayIndex];
        currentIndex += 64;
    }
    if (currentBits != 0)
    {
        *value = currentIndex + lowest_bit_index(currentBits);
        return true;
    }
    else
        return false;
}

static inline void intset_complement(intset_t* self, bool trim)
{
    size_t i;
    size_t domain = self->domain;

    if (domain > 0)
    {
        size_t arraySize = self->arraySize;
        uint64_t highMask = ~all_higher_bits(single_bit((domain + 63) & 63));

        for (i = 0; i < arraySize - 1; i++)
        {
            self->array[i] = (~self->array[i]) & UINT64_C(0xFFFFFFFFFFFFFFFF);
        }
        self->array[arraySize - 1] = (~self->array[arraySize - 1]) & highMask;
    }
}

static inline void intset_intersect(intset_t* self, const intset_t* pattern, bool trim)
{
    size_t i, arraySize = self->arraySize;
    for (i = 0; i < arraySize; i++)
    {
        self->array[i] &= pattern->array[i];
    }
}

static inline void intset_union(intset_t* self, const intset_t* pattern)
{
    size_t i, arraySize = self->arraySize;
    for (i = 0; i < arraySize; i++)
    {
        self->array[i] |= pattern->array[i];
    }
}

static inline void intset_remove_all(intset_t* self, const intset_t* pattern, bool trim)
{
    size_t i, arraySize = self->arraySize;
    for (i = 0; i < arraySize; i++)
    {
        self->array[i] &= ~pattern->array[i];
    }
}

static inline size_t intset_count(const intset_t* self)
{
    size_t i, arraySize = self->arraySize, sum = 0;
    for (i = 0; i < arraySize; i++)
    {
        sum += count_bits(self->array[i]);
    }
    return sum;
}

static inline size_t intset_count_until(const intset_t* self, size_t limit, size_t* w)
{
    size_t i, arraySize = self->arraySize, sum = 0;
    size_t theW = self->domain;
    for (i = 0; i < arraySize; i++)
    {
        uint64_t bits = self->array[i];
        if (bits)
        {
            sum += count_bits(bits);
            theW = i * 64 + bit_to_index(lowest_bit(bits)); /// any bit will do
            if (sum >= limit) break;
        }
    }
    *w = theW;
    return sum;
}

static inline size_t intset_count_common(const intset_t* self, const intset_t* pattern)
{
    size_t i, arraySize = self->arraySize, sum = 0;
    for (i = 0; i < arraySize; i++)
    {
        sum += count_bits(self->array[i] & pattern->array[i]);
    }
    return sum;
}

static inline size_t intset_count_common_until(const intset_t* self, const intset_t* pattern, size_t limit, size_t* w)
{
    size_t i, arraySize = self->arraySize, sum = 0;
    size_t theW = self->domain;
    for (i = 0; i < arraySize; i++)
    {
        uint64_t bits = self->array[i] & pattern->array[i];
        if (bits != 0)
        {
            sum += count_bits(bits);
            theW = i * 64 + bit_to_index(lowest_bit(bits)); /// any bit will do
            if (sum >= limit) break;
        }
    }
    *w = theW;
    return sum;
}

static inline size_t intset_count_common_inverse_until(const intset_t* self, const intset_t* pattern, size_t limit, size_t* w)
{
    size_t i, arraySize = self->arraySize, sum = 0;
    size_t theW = self->domain;
    for (i = 0; i < arraySize; i++)
    {
        uint64_t bits = self->array[i] & ~pattern->array[i];
        if (bits != 0)
        {
            sum += count_bits(bits);
            theW = i * 64 + bit_to_index(lowest_bit(bits)); /// any bit will do
            if (sum >= limit) break;
        }
    }
    *w = theW;
    return sum;
}

static inline void intset_swap_entries(intset_t* self, size_t i, size_t j, bool trim)
{
    bool temp = intset_get(self, i);
    intset_set(self, i, intset_get(self, j), trim);
    intset_set(self, j, temp, trim);
}

#endif
