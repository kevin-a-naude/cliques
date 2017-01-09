
#include "fatal.h"

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

void fatal_error(const char* message, ...)
{
    va_list argp;
    va_start(argp, message);
    vfprintf(stderr, message, argp);
    fprintf(stderr, "\n");
    va_end(argp);
    exit(EXIT_FAILURE);
}

void debug(const char* message, ...)
{
    va_list argp;
    va_start(argp, message);
    vfprintf(stderr, message, argp);
    va_end(argp);
}
