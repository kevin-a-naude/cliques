
#include <stdbool.h>
#include <stdint.h>

#include "adjacency.h"

int* dimacs_graph_stats(adjacency_t* bm, int* numIsolatedVertices, int* numPendantVertices, int* numUniqueDegree, double* degreeVariance, bool returnHistogram);

bool dimacs_load_graph(const char* filename, int* numVertices, int* numEdges, double* degreeVariance, adjacency_t* bm);
bool dimacs_load_graph_text(const char* filename, int* numVertices, int* numEdges, double* degreeVariance, adjacency_t* bm);
bool dimacs_load_graph_binary(const char* filename, int* numVertices, int* numEdges, double* degreeVariance, adjacency_t* bm);

bool dimacs_save_graph(const char* filename, const char* comment, const adjacency_t* bm);
bool dimacs_save_graph_text(const char* filename, const char* comment, const adjacency_t* bm);
bool dimacs_save_graph_binary(const char* filename, const char* comment, const adjacency_t* bm);
