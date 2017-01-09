
#include "new.h"
#include "adjacency.h"
#include "dimacs.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    if ((argc < 2) || ((argc == 2) && (strcmp(argv[1], "--stats") == 0)))
    {
        printf("Determines the degree sequence of a given graph.\n");
        printf("\n");
        printf("Usage:\n");
        printf("  %s [--stats] <input-dimacs-file>\n", "degseq");
    }
    else
    {
        size_t index = 1;
        bool stats = false;

        if (strcmp(argv[1], "--stats") == 0)
        {
            index++;
            stats = true;
        }

        if (file_exists(argv[index]))
        {
            int numVertices;
            int numEdges;
            adjacency_t adjacency;
            int* degrees;
            int numIsolated;
            int numPendants;
            int numUnique;
            double variance;
            bool first = true;

            adjacency_non(&adjacency);

            if (!dimacs_load_graph(argv[index], &numVertices, &numEdges, NULL, &adjacency))
                fatal_error("could not load graph: %s", argv[index]);

            degrees = dimacs_graph_stats(&adjacency, &numIsolated, &numPendants, &numUnique, &variance, true);

            if (stats)
            {
                printf("Num Isolated Vertices: %d\n", numIsolated);
                printf("Num Pendant/Leaf Vertices: %d\n", numPendants);
                printf("Num Vertices of Unique Degree: %d\n", numUnique);
                printf("Proportion with Unique Degree: %.6f\n", (double)numUnique / numVertices);
                printf("Variance(Degree): %.6f\n", variance);
                printf("\nDegree Sequence:\n\n");
            }

            for (int degree = numVertices; degree >= 0; degree--)
            {
                if (degrees[degree] > 0)
                {
                    if (!first) printf(" ");
                    first = false;
                    if (degrees[degree] == 1)
                        printf("%d", degree);
                    else
                        printf("%d:%d", degree, degrees[degree]);
                }
            }

            free_object(degrees);

            adjacency_destroy(&adjacency);
        }
        else
        {
            printf(" %s: file not found! ", argv[index]);
        }
        return 0;
    }
}
