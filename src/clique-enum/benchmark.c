
#include "benchmark.h"

#include "dimacs.h"
#include "intset.h"
#include "adjacency.h"
#include "timer.h"
#include "platform.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>

bool clique_enum_test(FILE* fout, const char* filename, clique_context_t* cc, uint64_t* numCliques, double* seconds)
{
    int numVertices, numEdges;
    adjacency_t bm;
    bool result = false;
    double degreeVariance;

    adjacency_non(&bm);

    if (dimacs_load_graph(filename, &numVertices, &numEdges, NULL, &bm))
    {
        timer_t timer;

        /* clear any self edges -- tut, tut */
        for (size_t i = 0; i < bm.n; i++)
        {
            adjacency_set(&bm, i, i, false);
        }

        timer_reset(&timer);
        timer_start(&timer);
        enumerate_cliques(cc, &bm);
        timer_stop(&timer);

        dimacs_graph_stats(&bm, NULL, NULL, NULL, &degreeVariance, false);

        fprintf(fout, "%s,%s,%d,%d,%.6f,%d,%"PRId64",%"PRId64",%.6f\n",
            cc->algorithm, os_basename(filename), numVertices, numEdges, degreeVariance, (int)cc->maxClique, cc->numCalls, cc->numCliques, timer.seconds);
        fflush(fout);
        *numCliques = cc->numCliques;
        *seconds = timer.seconds;
        result = true;
    }

    adjacency_destroy(&bm);
    return result;
}
