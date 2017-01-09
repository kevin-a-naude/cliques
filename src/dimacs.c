
#include "dimacs.h"

#include "fatal.h"

#include <stdio.h>
#include <string.h>

bool dimacs_process_text(const char* filename, void (*function)(void* obj, int i, int j), void* obj)
{
    FILE* file = fopen(filename, "r");
    if (file)
    {
        int ch, i, j;
        char line[1000];
        bool stop = false;

        while (!stop && ((ch = fgetc(file)) != EOF))
        {
            switch (ch)
            {
                case 'e':
                    if (2 != fscanf(file, "%d %d", &i, &j))
                        fatal_error("invalid DIMACS text file");
                    else
                    {
                        function(obj, i, j);
                    }
                    break;

                case '\n':
                    break;

                default:
                    if (fgets(line, sizeof(line), file) == NULL) stop = true;
                    break;
            }
        }

        fclose(file);
        return true;
    }
    else
        return false;
}

bool dimacs_process_binary(const char* filename, int* numVertices, void (*function)(void* obj, int i, int j), void* obj)
{
    FILE* file = fopen(filename, "rb");
    if (file)
    {
        int ch, i, j, k, preamble, rowSize, rowIndex;
        size_t len;
        char line[1000];

        if (1 != fscanf(file, "%d", &preamble)) goto FATAL;
        if (preamble < 0) goto FATAL;

        if (NULL == fgets(line, sizeof(line), file)) goto FATAL;
        len = strlen(line);
        while (line[len - 1] != '\n')
        {
            if (NULL == fgets(line, sizeof(line), file)) goto FATAL;
            len = strlen(line);
        }

        fseek(file, preamble, SEEK_CUR);

        bool done = false;
        for (i = 0; !done; i++)
        {
            rowSize = (i + 8) / 8;
            j = 0;

            for (rowIndex = 0; rowIndex < rowSize; rowIndex++)
            {
                ch = fgetc(file);
                if (feof(file))
                {
                    done = true;
                    break;
                }

                if (function != NULL)
                {
                    for (k = 0; (k < 8) && (j <= i); k++, j++)
                    {
                        int bit = (1 << (7 - k));
                        if ((ch & bit) != 0)
                        {
                            function(obj, i + 1, j + 1);
                        }
                    }
                }
            }
            if (done) break;
        }

        *numVertices = i;

        fclose(file);
        return true;
    }
    else
        return false;

FATAL:
    fatal_error("invalid DIMACS binary file");
    return false;
}

void dimacs_highest_vertex_id(void* obj, int i, int j)
{
    int* n = (int*) obj;
    if (i > *n) *n = i;
    if (j > *n) *n = j;
}

void dimacs_fill_matrix(void* obj, int i, int j)
{
    adjacency_t* matrix = (adjacency_t*) obj;
    adjacency_symmetric_set(matrix, i-1, j-1, true);
}

int* dimacs_graph_stats(adjacency_t* bm, int* numIsolatedVertices, int* numPendantVertices, int* numUniqueDegree, double* degreeVariance, bool returnHistogram)
{
    int numVertices;
    int* degrees;
    int numIsolated;
    int numPendants;
    int numUnique;
    double meanDegree, variance;

    numVertices = bm->n;
    degrees = new_array(sizeof(int), numVertices + 1);

    meanDegree = 0.0;
    for (int v = 0; v < numVertices; v++)
    {
        int degree = intset_count(adjacency_row(bm, v));
        meanDegree += degree;
        degrees[degree]++;
    }
    meanDegree /= numVertices;

    numUnique = 0;
    variance = 0.0;
    for (int degree = 0; degree < numVertices + 1; degree++)
    {
        if (degrees[degree] > 0)
        {
            double contrib = (degree - meanDegree) * (degree - meanDegree);
            variance += contrib * degrees[degree];
        }
        if (degrees[degree] == 1) numUnique++;
    }
    variance /= numVertices;
    numIsolated = degrees[0];
    numPendants = degrees[1];

    if (numIsolatedVertices) *numIsolatedVertices = numIsolated;
    if (numPendantVertices) *numPendantVertices = numPendants;
    if (numUniqueDegree) *numUniqueDegree = numUnique;
    if (degreeVariance) *degreeVariance = variance;

    if (!returnHistogram) degrees = free_object(degrees);
    return degrees;
}

bool dimacs_load_graph_text(const char* filename, int* numVertices, int* numEdges, double* degreeVariance, adjacency_t* bm)
{
    *numVertices = 0;
    if (!dimacs_process_text(filename, &dimacs_highest_vertex_id, numVertices)) return false;
    adjacency_resize(bm, *numVertices);
    adjacency_clear(bm);
    if (!dimacs_process_text(filename, &dimacs_fill_matrix, bm)) return false;
    *numEdges = (int)(adjacency_count(bm) / 2);
    if (degreeVariance != NULL) dimacs_graph_stats(bm, NULL, NULL, NULL, degreeVariance, false);
    return true;
}

bool dimacs_load_graph_binary(const char* filename, int* numVertices, int* numEdges, double* degreeVariance, adjacency_t* bm)
{
    int tmp;
    *numVertices = 0;
    if (!dimacs_process_binary(filename, numVertices, NULL, NULL)) return false;
    adjacency_resize(bm, *numVertices);
    adjacency_clear(bm);
    if (!dimacs_process_binary(filename, &tmp, &dimacs_fill_matrix, bm)) return false;
    *numEdges = (int)(adjacency_count(bm) / 2);
    if (degreeVariance != NULL) dimacs_graph_stats(bm, NULL, NULL, NULL, degreeVariance, false);
    return true;
}

bool dimacs_load_graph(const char* filename, int* numVertices, int* numEdges, double* degreeVariance, adjacency_t* bm)
{
    size_t len = strlen(filename);
    if ((len >= 2) && (filename[len - 2] == '.') && (filename[len - 1] == 'b'))
        return dimacs_load_graph_binary(filename, numVertices, numEdges, degreeVariance, bm);
    else
        return dimacs_load_graph_text(filename, numVertices, numEdges, degreeVariance, bm);
}


bool dimacs_save_graph(const char* filename, const char* comment, const adjacency_t* bm)
{
    size_t len = strlen(filename);
    if ((len >= 2) && (filename[len - 2] == '.') && (filename[len - 1] == 'b'))
        return dimacs_save_graph_binary(filename, comment, bm);
    else
        return dimacs_save_graph_text(filename, comment, bm);
}

bool dimacs_save_graph_text(const char* filename, const char* comment, const adjacency_t* bm)
{
    FILE* fout;

    fout = fopen(filename, "w");
    if (fout)
    {
        size_t n = bm->n;
        fputs(comment, fout);
        for (size_t i = 0; i < n; i++)
        {
            for (size_t j = i + 1; j < n; j++)
            {
                if (adjacency_get(bm, i, j))
                {
                    fprintf(fout, "e %d %d\n", (int)(i + 1), (int)(j + 1));
                }
            }
        }

        fclose(fout);
        return true;
    }
    else
        return false;
}

bool dimacs_save_graph_binary(const char* filename, const char* comment, const adjacency_t* bm)
{
    FILE* fout;

    fout = fopen(filename, "wb");
    if (fout)
    {
        size_t n = bm->n;
        size_t len = strlen(comment);
        fprintf(fout, "%d\n", (int) len);
        if (len > 0)
        {
            fputs(comment, fout);
        }

        for (size_t i = 0; i < n; i++)
        {
            size_t rowSize = (i + 8) / 8;
            size_t j = 0;

            for (size_t rowIndex = 0; rowIndex < rowSize; rowIndex++)
            {
                int ch = 0;

                for (size_t k = 0; (k < 8) && (j <= i); k++, j++)
                {
                    int bit = (1 << (7 - k));
                    if (adjacency_get(bm, i, j)) ch |= bit;
                }
                fputc(ch, fout);
            }
        }

        fclose(fout);
        return true;
    }
    else
        return false;
}
