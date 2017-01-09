
#pragma once

#ifndef CLIQUE_ENUM_H
#define CLIQUE_ENUM_H 1

#include "intset.h"
#include "adjacency.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct __clique_context_t clique_context_t;

typedef void(*clique_enum_start_function)(struct __clique_context_t* ctx);

struct __clique_context_t
{
    size_t n;

    const char* algorithm;

    size_t maxClique;
    uint64_t numCalls;
    uint64_t numCliques;

    adjacency_t* matrix;
    setmanager_t manager;

    clique_enum_start_function start;
};

void clique_context(clique_context_t* self, clique_enum_start_function start);
void clique_context_destroy(clique_context_t* self);

void enumerate_cliques(clique_context_t* self, adjacency_t* adjacency);

void clique_enum_permute_high_degree_first(adjacency_t* adjacency, setmanager_t* manager);

#endif
