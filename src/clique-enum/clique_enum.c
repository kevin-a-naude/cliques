
#include "clique_enum.h"

#include <stdlib.h>

void clique_context(clique_context_t* self, clique_enum_start_function start)
{
    self->n = 0;

    self->maxClique = 0;
    self->numCalls = 0;
    self->numInPivot = 0;
    self->numCliques = 0;

    self->matrix = NULL;

    self->start = start;
}

void clique_context_destroy(clique_context_t* self)
{
}

void enumerate_cliques(clique_context_t* self, adjacency_t* adjacency)
{
    self->n = adjacency->n;
    self->maxClique = 0;
    self->numCalls = 0;
    self->numInPivot = 0;
    self->numCliques = 0;

    self->matrix = adjacency;

    setmanager(&self->manager, self->n);
    self->start(self);
    setmanager_destroy(&self->manager);
}

void clique_enum_permute_high_degree_first_in_place(adjacency_t* m, setmanager_t* manager)
{
    adjacency_t* bm = m;
    size_t n = bm->n;
    size_t* positionOf = new_array(sizeof(size_t), n);
    size_t* valueAt = new_array(sizeof(size_t), n);
    size_t* permutation = new_array(sizeof(size_t), n);
    size_t* degree = new_array(sizeof(size_t), n);
    intset_t* avail;

    for (size_t index = 0; index < n; index++)
    {
        degree[index] = intset_count(adjacency_row(bm, index)) / 2;
        positionOf[index] = index;
        valueAt[index] = index;
    }

    avail = intset(n, true, manager);

    for (size_t index = n; index > 0; index--)
    {
        size_t v;
        size_t i = index - 1;
        size_t highestDegree = 0;
        size_t indexOfHighest = n;

        intset_walk_t it;
        intset_walk(&it, avail);

        while (intset_walk_next(&it, &v))
        {
            if ((indexOfHighest == n) || (degree[v] >= highestDegree))
            {
                highestDegree = degree[v];
                indexOfHighest = v;
            }
        }
        intset_remove(avail, indexOfHighest, false);
        permutation[(n - 1) - i] = indexOfHighest;
    }
    free_object(degree);

    /* now we must apply the permutation */
    for (size_t index = 0; index < n; index++)
    {
        size_t toMove;
        if (permutation[index] == positionOf[index]) continue;

        /* migrate the permutation entries and associated the vertices*/
        toMove = valueAt[index];

        positionOf[toMove] = positionOf[permutation[index]];
        valueAt[positionOf[toMove]] = toMove;

        positionOf[permutation[index]] = index;
        valueAt[index] = permutation[index];

        adjacency_symmetric_swap(bm, index, positionOf[toMove]);

    }

    free_object(permutation);
    free_object(valueAt);
    free_object(positionOf);
}


typedef struct
{
    size_t index;
    size_t degree;
} vertex_t;

int clique_enum_vertex_order(const void * e1, const void * e2)
{
    vertex_t u = *((vertex_t*)e1);
    vertex_t v = *((vertex_t*)e2);
    if (u.degree > v.degree)
        return -1;
    else
    if (u.degree < v.degree)
        return +1;
    else
    if (u.index < v.index)
        return -1;
    else
    if (u.index > v.index)
        return +1;
    else
        return 0;
}

void clique_enum_permute_high_degree_first(adjacency_t* m, setmanager_t* manager)
{
    adjacency_t* bm = m;
    adjacency_t bmNew;
    size_t n = bm->n;
    vertex_t* info = new_array(sizeof(vertex_t), n);
    vertex_t* infoInverse = new_array(sizeof(vertex_t), n);

    for (size_t index = 0; index < n; index++)
    {
        info[index].degree = intset_count(adjacency_row(bm, index));
        info[index].index = index;
    }

    qsort(info, n, sizeof(vertex_t), clique_enum_vertex_order);

    for (size_t index = 0; index < n; index++)
    {
        vertex_t v = info[index];
        infoInverse[v.index].index = index;
        infoInverse[v.index].degree = v.degree;
    }

    adjacency(&bmNew, n);
    for (size_t u = 0; u < n; u++)
    {
        size_t v;
        vertex_t ut = info[u];
        intset_walk_t it;
        intset_walk(&it, adjacency_row(bm, ut.index));
        while (intset_walk_next(&it, &v))
        {
            vertex_t vt = infoInverse[v];
            adjacency_set(&bmNew, u, vt.index, true);
        }
    }

    free_object(infoInverse);
    free_object(info);

    adjacency_destroy(bm);
    *m = bmNew;
}

