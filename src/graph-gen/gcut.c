
#include "adjacency.h"
#include "dimacs.h"
#include "fatal.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>

int main(int argc, const char* argv[])
{
    if (argc < 5)
    {
        printf("Generates a graph with a cut vertex separating two cliques.\n");
        printf("\n");
        printf("Usage:\n");
        printf("  %s <output-file-prefix> <output-file-suffix> <num-clusters> <cluster-size>\n", "Gcut");
    }
    else
    {
        FILE* fout;
        char filename[200];
        char comment[200];
        adjacency_t bm;

        int value = 0;
        size_t m = 0;
        size_t n = 0;

        if ((sscanf(argv[3], "%d", &value) == 1) && (value > 0))
            n = (size_t)value;
        else
            fatal_error("invalid parameter 'num-clusters': %s", argv[3]);

        if ((sscanf(argv[4], "%d", &value) == 1) && (value > 0))
            m = (size_t)value;
        else
            fatal_error("invalid parameter 'cluster-size': %s", argv[4]);

        sprintf(filename, "%s%d_%d%s", argv[1], (int)n, (int)m, argv[2]);
        sprintf(comment, "c %d cliques of size %d around a hub vertex\n", (int)n, (int)n);

        /*
        adjacency(&bm, m * n + (n - 1));
        adjacency_clear(&bm);

        for (size_t k = 0; k < n; k++)
        {
            for (size_t i = k*m; i < (k + 1)*m; i++)
            {
                for (size_t j = k*m; j < (k + 1)*m; j++)
                {
                    if (i != j) adjacency_symmetric_set(&bm, i, j, true);
                }
            }
        }
        for (size_t k = 1; k < n; k++)
        {
            adjacency_symmetric_set(&bm, (k-1)*m, m * n + k - 1, true);
            adjacency_symmetric_set(&bm, k*m, m * n + k - 1, true);
        }
        */

        adjacency(&bm, m * n + 1);
        adjacency_clear(&bm);

        for (size_t k = 0; k < n; k++)
        {
            for (size_t i = k*m; i < (k+1)*m; i++)
            {
                for (size_t j = k*m; j < (k+1)*m; j++)
                {
                    if (i != j) adjacency_symmetric_set(&bm, i, j, true);
                }
            }
            adjacency_symmetric_set(&bm, k*m, m*n, true);
        }

        if (!dimacs_save_graph(filename, comment, &bm))
            fatal_error("Could not write to file: %s", filename);

        adjacency_destroy(&bm);

        return 0;
    }
}
