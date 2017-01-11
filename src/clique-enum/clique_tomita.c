
#include "clique_tomita.h"

intset_t* clique_enum_tomita_pivot(clique_context_t* ctx, intset_t* p, intset_t* x, intset_t* r)
{
    size_t most = 0;
    size_t q = 0;

    size_t v;
    intset_walk_t it;
    intset_t *nv, *Q;

    intset_walk(&it, x);
    while (intset_walk_next(&it, &v))
    {
        nv = adjacency_row(ctx->matrix, v);
        size_t count = intset_count_common(p, nv) + 1;
        if (count > most)
        {
            most = count;
            q = v;
        }
    }

    intset_walk(&it, p);
    while (intset_walk_next(&it, &v))
    {
        nv = adjacency_row(ctx->matrix, v);
        size_t count = intset_count_common(p, nv) + 1;
        if (count > most)
        {
            most = count;
            q = v;
        }
    }

    nv = adjacency_row(ctx->matrix, q);
    Q = intset_copy_remove(p, nv);
    return Q;
}

void clique_enum_tomita_apply(clique_context_t* self, intset_t* p, intset_t* x, intset_t* r)
{
    intset_t* q;
    self->numCalls++;
    if (!intset_is_empty(p) && (q = clique_enum_tomita_pivot(self, p, x, r)))
    {
        size_t v;
        intset_walk_t it;
        intset_t *p2, *x2, *nv;

        intset_walk(&it, q);
        while (intset_walk_next(&it, &v))
        {
            intset_remove(p, v, false);

            nv = adjacency_row(self->matrix, v);

            p2 = intset_copy_intersect(p, nv);
            x2 = intset_copy_intersect(x, nv);

            intset_add(r, v);
            clique_enum_tomita_apply(self, p2, x2, r);
            intset_remove(r, v, true);

            intset_recycle(x2);
            intset_recycle(p2);

            intset_add(x, v);
        }

        intset_recycle(q);
    }
    else
    {
        if (intset_is_empty(x))
        {
            size_t size = intset_count(r);
            self->numCliques++;
            if (self->maxClique < size) self->maxClique = size;
        }
    }
}

void clique_enum_tomita_start(clique_context_t* self)
{
    intset_t* r = intset(self->n, false, &self->manager);
    intset_t* p = intset(self->n, true, &self->manager);
    intset_t* x = intset(self->n, false, &self->manager);

    clique_enum_tomita_apply(self, p, x, r);

    intset_recycle(x);
    intset_recycle(p);
    intset_recycle(r);
}

void clique_context_tomita(clique_context_t* self)
{
    clique_context(self, clique_enum_tomita_start);
    self->algorithm = "tomita";
}
