
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

int main(int argc, const char* argv[])
{
    if (argc < 6)
    {
        printf("Generates random graph by Gnp model.\n");
        printf("\n");
        printf("Usage:\n");
        printf("  %s <output-file-prefix> <output-file-suffix> n p count [seed]\n", "Gnp");
    }
    else
    {
        int value = 0;
        size_t n = 0;
        double p = 0.0;
        size_t count = 0;
        uint32_t seed = 0;
        prng_t prng;

        if ((sscanf(argv[3], "%d", &value) == 1) && (value > 0))
            n = (size_t)value;
        else
            fatal_error("invalid parameter n: %s", argv[3]);

        if ((sscanf(argv[4], "%lf", &p) == 1) && (p > 0.0) && (p < 1.0)) {}
        else
            fatal_error("invalid parameter p: %s", argv[4]);

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

        for (size_t k = 1; k <= count; k++)
        {
            adjacency_t bm;
            char filename[200];
            char comment[200];

            sprintf(filename, "%s%d_%s_%d%s", argv[1], (int)n, argv[4], (int)k, argv[2]);
            sprintf(comment, "c graph generated according to Erdos-Renyi Gnp model, with n = %d and p = %s\n", (int)n, argv[4]);

            adjacency(&bm, n);
            adjacency_clear(&bm);
            for (size_t i = 0; i < n; i++)
            {
                for (size_t j = i + 1; j < n; j++)
                {
                    if (prng_double(&prng) < p)
                    {
                        adjacency_symmetric_set(&bm, i, j, true);
                    }
                }
            }

            if (!dimacs_save_graph(filename, comment, &bm))
                fatal_error("Could not write to file: %s", filename);

            adjacency_destroy(&bm);
        }
        return 0;
    }
}
