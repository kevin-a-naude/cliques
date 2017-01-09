
#pragma once

#ifndef ADJACENCY_H
#define ADJACENCY_H 1

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "new.h"
#include "arena.h"
#include "intset.h"

typedef struct {
    size_t n;
    intset_t** rows;
    setmanager_t manager;
} adjacency_t;

/* adjacency_t */

static inline void adjacency(adjacency_t* self, size_t n)
{
    size_t i;
    setmanager(&self->manager, n);
    self->n = n;
    self->rows = new_array(sizeof(intset_t*), n);
    for (i = 0; i < n; i++)
    {
        self->rows[i] = intset(n, false, &self->manager);
    }
}

static inline void adjacency_non(adjacency_t* self)
{
    setmanager(&self->manager, 1);
    self->rows = NULL;
}

static inline void adjacency_resize(adjacency_t* self, size_t n)
{
    if ((n != self->n) || (self->rows == NULL))
    {
        size_t i;
        intset_t** temp = self->rows;
        size_t k = (self->rows == NULL)? 0 : self->n;
        size_t m = (self->rows == NULL)? 0 : (n < self->n)? n : self->n;
        self->n = n;
        self->rows = new_array(sizeof(intset_t*), n);

        setmanager_repurpose(&self->manager, n);

        for (i = 0; i < n; i++)
        {
            self->rows[i] = intset(n, false, &self->manager);
        }

        free_object(temp);
    }
}

static inline void adjacency_destroy(adjacency_t* self)
{
    if (self->rows)
    {
        self->rows = free_object(self->rows);
    }
    setmanager_destroy(&self->manager);
}

static inline void adjacency_clear(adjacency_t* self)
{
    size_t i, n = self->n;
    for (i = 0; i < n; i++)
    {
        intset_clear(self->rows[i], true);
    }
}

static inline void adjacency_match_size(adjacency_t* self, const adjacency_t* pattern)
{
    if ((self->rows == NULL) || (self->n != pattern->n))
    {
        adjacency_destroy(self);
        adjacency(self, pattern->n);
    }
}

static inline void adjacency_complement(adjacency_t* self)
{
    size_t i, n = self->n;
    for (i = 0; i < n; i++)
    {
        intset_complement(self->rows[i], true);
    }
}

static inline void adjacency_copy(adjacency_t* self, const adjacency_t* pattern)
{
    size_t i, n;
    adjacency_match_size(self, pattern);
    n = self->n;
    for (i = 0; i < n; i++)
    {
        intset_recycle(self->rows[i]);
        self->rows[i] = intset_copy(pattern->rows[i], &self->manager);
    }
}

static inline bool adjacency_nonzero_diagonal(adjacency_t* self)
{
    size_t i, n = self->n;
    for (i = 0; i < n; i++)
    {
        if (intset_contains(self->rows[i], i)) return true;
    }
    return false;
}

static inline bool adjacency_get(const adjacency_t* self, size_t i, size_t j)
{
    return intset_get(self->rows[i], j);
}

static inline void adjacency_set(adjacency_t* self, size_t i, size_t j, bool enabled)
{
    intset_set(self->rows[i], j, enabled, true);
}

static inline intset_t* adjacency_row(adjacency_t* self, size_t i)
{
    return self->rows[i];
}

static inline void adjacency_symmetric_set(adjacency_t* self, size_t i, size_t j, bool enabled)
{
    intset_set(self->rows[i], j, enabled, true);
    intset_set(self->rows[j], i, enabled, true);
}

static inline void adjacency_swap_rows(adjacency_t* self, size_t i1, size_t i2)
{
    intset_t* temp = self->rows[i1];
    self->rows[i1] = self->rows[i2];
    self->rows[i2] = temp;
}

static inline void adjacency_swap_columns(adjacency_t* self, size_t j1, size_t j2)
{
    if (j1 != j2)
    {
        size_t i, n = self->n;
        for (i = 0; i < n; i++)
        {
            intset_swap_entries(self->rows[i], j1, j2, true);
        }
    }
}

static inline void adjacency_symmetric_swap(adjacency_t* self, size_t i, size_t j)
{
    adjacency_swap_rows(self, i, j);
    adjacency_swap_columns(self, i, j);
}

static inline size_t adjacency_count(adjacency_t* self)
{
    size_t i, n = self->n, sum = 0;
    for (i = 0; i < n; i++)
    {
        sum += intset_count(self->rows[i]);
    }
    return sum;
}

#endif
