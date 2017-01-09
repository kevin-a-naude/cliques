
#include "new.h"
#include "adjacency.h"
#include "dimacs.h"
#include "random.h"

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

typedef struct
{
    int i, j;
} entry_t;

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

void shuffle(entry_t* array, size_t count, prng_t* prng)
{
    while (count > 1)
    {
        size_t pos = prng_uint32_capped(prng, count);
        if (pos < count)
        {
            entry_t temp = array[pos];
            array[pos] = array[count - 1];
            array[count - 1] = temp;
        }
        count--;
    }
}

int main(int argc, const char* argv[])
{
    if (argc < 6)
    {
        printf("Generates random graph by Gnm model.\n");
        printf("\n");
        printf("Usage:\n");
        printf("  %s <output-file-prefix> <output-file-suffix> n m count [seed]\n", "Gnm");
    }
    else
    {
        int value = 0;
        size_t n = 0;
        size_t m = 0;
        size_t maxEdges = 0;
        size_t index = 0;
        size_t count = 0;
        uint32_t seed = 0;
        entry_t* entries = NULL;
        prng_t prng;

        if ((sscanf(argv[3], "%d", &value) == 1) && (value > 1))
            n = (size_t)value;
        else
            fatal_error("invalid parameter n: %s", argv[3]);

        if ((sscanf(argv[4], "%d", &value) == 1) && (value >= 0))
            m = (size_t)value;
        else
            fatal_error("invalid parameter m: %s", argv[4]);

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

        maxEdges = n * (n - 1) / 2;
        entries = new_array(sizeof(entry_t), maxEdges);
        index = 0;
        for (size_t i = 0; i < n; i++)
        {
            for (size_t j = i + 1; j < n; j++)
            {
                entries[index].i = i;
                entries[index].j = j;
                index++;
            }
        }

        for (size_t k = 1; k <= count; k++)
        {
            adjacency_t bm;
            char filename[200];
            char comment[200];

            shuffle(entries, maxEdges, &prng);

            sprintf(filename, "%s%d_%d_%d%s", argv[1], (int)n, (int)m, (int)k, argv[2]);
            sprintf(comment, "c graph generated according to Erdos-Renyi Gnm model, with n = %d and m = %d\n", (int)n, (int)m);

            adjacency(&bm, n);
            adjacency_clear(&bm);
            for (size_t e = 0; e < m; e++)
            {
                entry_t edge = entries[e];
                adjacency_symmetric_set(&bm, edge.i, edge.j, true);
            }

            if (!dimacs_save_graph(filename, comment, &bm))
                fatal_error("Could not write to file: %s", filename);

            adjacency_destroy(&bm);
        }

        free_object(entries);
        return 0;
    }
}
