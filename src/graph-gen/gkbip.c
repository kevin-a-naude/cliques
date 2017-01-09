
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
        printf("Generates complete bipartite graphs.\n");
        printf("\n");
        printf("Usage:\n");
        printf("  %s <output-file-prefix> <output-file-suffix> <m> <n>\n", "Gkbip");
    }
    else
    {
        char filename[200];
        char comment[200];
        adjacency_t bm;

        int value = 0;
        size_t m = 0;
        size_t n = 0;

        if ((sscanf(argv[3], "%d", &value) == 1) && (value > 0))
            m = (size_t)value;
        else
            fatal_error("invalid parameter 'm': %s", argv[3]);

        if ((sscanf(argv[4], "%d", &value) == 1) && (value > 0))
            n = (size_t)value;
        else
            fatal_error("invalid parameter 'n': %s", argv[4]);

        sprintf(filename, "%s%d_%d%s", argv[1], (int)m, (int)n, argv[2]);
        sprintf(comment, "c %d x %d complete bipartite graph\n", (int)m, (int)n);

        adjacency(&bm, m + n);
        adjacency_clear(&bm);

        for (size_t i = 0; i < m; i++)
        {
            for (size_t j = 0; j < n; j++)
            {
                adjacency_symmetric_set(&bm, i, m + j, true);
            }
        }

        if (!dimacs_save_graph(filename, comment, &bm))
            fatal_error("Could not write to file: %s", filename);

        adjacency_destroy(&bm);

        return 0;
    }
}
