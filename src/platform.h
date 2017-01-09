
#pragma once

#ifndef PLATFORM_H
#define PLATFORM 1

#include <stdio.h>

const char* os_basename(const char* filename);

long os_getline(char **lineptr, size_t *n, FILE *stream);

char* os_trim(char *str);

char* os_trim_newline(char *str);

#endif
