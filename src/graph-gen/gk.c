
#include "intset.h"
#include "adjacency.h"
#include "dimacs.h"
#include "random.h"

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

bool file_exists(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (file)
    {
        fclose(file);
        return true;
    }
    else
        return false;
}

typedef struct
{
    size_t u, v;
} edge_t;


void swap(int i, int j, edge_t* list)
{
    edge_t t = list[i];
    list[i] = list[j];
    list[j] = t;
}

void swapuv(edge_t* item)
{
    int t = item->u;
    item->u = item->v;
    item->v = t;
}

bool list_contains(edge_t* list, int count, edge_t e)
{
    for (int c = 0; c < count; c++)
    {
        if ((list[c].u == e.u) && (list[c].v == e.v))
            return true;
    }
    return false;
}

void generate_regular(adjacency_t* bm, size_t n, size_t k, prng_t* prng)
{
    size_t index;
    size_t edge_count = n * k / 2;
    size_t m = k / 2;
    size_t x = k - 2 * m;
    edge_t* list = new_array(sizeof(edge_t), edge_count);
    intset_t* listv;

    setmanager_t manager;

    setmanager(&manager, n);

    listv = intset(n * n, false, &manager);

    adjacency_resize(bm, n);
    adjacency_clear(bm);

    index = 0;
    for (size_t u = 0; u < n; u++)
    {
        for (size_t d = 0; d <= 2 * m; d++)
        {
            size_t w = (u + d - m + n) % n;

            if (w == u)
            {
                if (x == 0)
                    continue;
                else
                    w = (w + (n / 2)) % n;
            }

            if (u < w)
            {
                list[index].u = u;
                list[index].v = w;
                intset_add(listv, u * n + w);
                index++;
            }
        }
    }

    for (size_t stir = 0; stir < 100; stir++)
    {
        for (index = edge_count; index > 1; index--)
        {
            size_t v1, v2;
            uint32_t i = prng_uint32_capped(prng, index);
            uint32_t j = prng_uint32_capped(prng, index - 1);
            swap(i, index - 1, list);
            swap(j, index - 2, list);
            v1 = list[index - 1].v;
            v2 = list[index - 2].v;
            if ((v1 != list[index - 2].u) && (v2 != list[index - 1].u))
            {
                edge_t e1, e2;
                e1.u = list[index - 1].u;
                e1.v = v2;
                e2.u = list[index - 2].u;
                e2.v = v1;
                if (e1.u > e1.v) swapuv(&e1);
                if (e2.u > e2.v) swapuv(&e2);
                if (!intset_get(listv, e1.u * n + e1.v) && !intset_get(listv, e2.u * n + e2.v))
                {
                    intset_remove(listv, list[index - 1].u * n + list[index - 1].v, false);
                    intset_remove(listv, list[index - 2].u * n + list[index - 2].v, false);
                    intset_add(listv, e1.u * n + e1.v);
                    intset_add(listv, e2.u * n + e2.v);
                    list[index - 1] = e1;
                    list[index - 2] = e2;
                }
            }
        }
    }

    for (index = edge_count; index > 0; index--)
    {
        edge_t e = list[index - 1];
        adjacency_symmetric_set(bm, e.u, e.v, true);
    }

    intset_recycle(listv);
    setmanager_destroy(&manager);
    free_object(list);
}

int main(int argc, const char* argv[])
{
    if (argc < 6)
    {
        printf("Generates random regular graphs.\n");
        printf("\n");
        printf("Usage:\n");
        printf("  %s <output-file-prefix> <output-file-suffix> <n> <k> <count> [seed]\n", "Gk");
    }
    else
    {
        int value = 0;
        size_t n = 0;
        size_t k = 0;
        size_t count = 0;
        uint32_t seed = 0;
        prng_t prng;

        if ((sscanf(argv[3], "%d", &value) == 1) && (value > 0))
            n = (size_t)value;
        else
            fatal_error("invalid parameter n: %s", argv[3]);

        if ((sscanf(argv[4], "%d", &value) == 1) && (value > 0))
            k = (size_t)value;
        else
            fatal_error("invalid parameter k: %s", argv[4]);

        if ((sscanf(argv[5], "%d", &value) == 1) && (value > 0))
            count = (size_t)value;
        else
            fatal_error("invalid parameter 'count': %s", argv[5]);

        if (argc >= 7)
        {
            if (sscanf(argv[6], "%"SCNu32, &seed) != 1)
                fatal_error("invalid parameter 'seed': %s", argv[6]);
            prng_seed(&prng, seed);
        }
        else
        {
            prng_init(&prng);
        }

        for (size_t c = 1; c <= count; c++)
        {
            adjacency_t bm;
            char filename[200];
            char comment[200];

            sprintf(filename, "%s%d_%d_%d%s", argv[1], (int)n, (int)k, (int)c, argv[2]);
            sprintf(comment, "c k-regular random graph, with n = %d and k = %d\n", (int)n, (int)k);

            adjacency(&bm, n);

            generate_regular(&bm, n, k, &prng);

            if (!dimacs_save_graph(filename, comment, &bm))
                fatal_error("Could not write to file: %s", filename);

            adjacency_destroy(&bm);
        }
        return 0;
    }
}
