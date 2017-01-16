
#include "clique_naude.h"

intset_t* clique_enum_naude_pivot_extra(clique_context_t* ctx, intset_t* p, intset_t* x, intset_t* r, intset_t** extra)
{
    size_t v, q, least;
    intset_walk_t it;
    intset_t *nv, *nw, *Q, *ex = NULL;
    uint64_t numInPivot = ctx->numInPivot;
    size_t domain = ctx->n;

search:
    q = domain; // an initial value which is not a valid vertex
    least = domain + 1; // not infinity, but large enough

    intset_walk(&it, x);
    while (intset_walk_next(&it, &v))
    {
        size_t w = 0, count;
        nv = adjacency_row(ctx->matrix, v);
        count = intset_count_common_inverse_until(p, nv, least, &w);
        if (count < least)
        {
            if (count <= 2)
            {
                if (count != 1) // count in { 0, 2 }
                {
                    q = v;
                    goto conclude;
                }
                else
                {
                    /// Process w in place
                    nw = adjacency_row(ctx->matrix, w);
                    if (ex == NULL)
                    {
                        ex = intset(domain, false, &ctx->manager);
                    }
                    numInPivot++;
                    intset_add(ex, w);
                    intset_add(r, w);
                    intset_intersect(p, nw, false);
                    intset_intersect(x, nw, false);

                    if ((q < domain) && !intset_contains(nw, q)) goto search;
                }
            }
            else
            {
                q = v;
                least = count;
            }
        }
    }

    intset_walk(&it, p);
    while (intset_walk_next(&it, &v))
    {
        size_t w = 0, count;
        nv = adjacency_row(ctx->matrix, v);
        count = intset_count_common_inverse_until(p, nv, least, &w);
        if (count < least)
        {
            if (count <= 2)
            {
                if (count != 1) // count in { 0, 2 }
                {
                    q = v;
                    goto conclude;
                }
                else
                {
                    /// Process v in place
                    if (ex == NULL)
                    {
                        ex = intset(domain, false, &ctx->manager);
                    }
                    numInPivot++;
                    intset_add(ex, v);
                    intset_add(r, v);
                    intset_intersect(p, nv, false);
                    intset_intersect(x, nv, false);

                    if ((q < domain) && !intset_contains(nv, q)) goto search;
                }
            }
            else
            {
                q = v;
                least = count;
            }
        }
    }

conclude:
    ctx->numInPivot = numInPivot;
    *extra = ex;
    if (q < domain)
    {
        nv = adjacency_row(ctx->matrix, q);
        Q = intset_copy_remove(p, nv);
        return Q;
    }
    else
    {
        return NULL;
    }
}

void clique_enum_naude_apply(clique_context_t* self, intset_t* p, intset_t* x, intset_t* r)
{
    intset_t *q, *extra = NULL;
    self->numCalls++;
    if (!intset_is_empty(p) && (q = clique_enum_naude_pivot_extra(self, p, x, r, &extra)))
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
            clique_enum_naude_apply(self, p2, x2, r);
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
    if (extra != NULL)
    {
        intset_remove_all(r, extra, true);
        intset_recycle(extra);
    }
}

void clique_enum_naude_start(clique_context_t* self)
{
    intset_t* r = intset(self->n, false, &self->manager);
    intset_t* p = intset(self->n, true, &self->manager);
    intset_t* x = intset(self->n, false, &self->manager);

    clique_enum_naude_apply(self, p, x, r);

    intset_recycle(x);
    intset_recycle(p);
    intset_recycle(r);
}

void clique_context_naude(clique_context_t* self)
{
    clique_context(self, clique_enum_naude_start);
    self->algorithm = "naude";
}
