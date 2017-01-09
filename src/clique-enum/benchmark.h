
#pragma once

#ifndef BENCHMARK_H
#define BENCHMARK 1

#include "clique_enum.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

bool clique_enum_test(FILE* fout, const char* filename, clique_context_t* cc, uint64_t* numCliques, double* seconds);

#endif
