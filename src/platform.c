
#include "platform.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* a somewhat portable implementations of POSIX functions that are not universally available */

const char* os_basename(const char* filename)
{
    size_t len = strlen(filename);
    while ((len > 0) && (filename[len - 1] != '\\') && (filename[len - 1] != '/') && (filename[len - 1] != ':'))
    {
        len--;
    }
    return &filename[len];
}

long os_getline(char **lineptr, size_t *n, FILE *stream)
{
    char *line = NULL, *rline = NULL;
    size_t size, read;
    int c;

    if (!lineptr || !stream || !n)
    {
        return -1;
    }

    if ((c = fgetc(stream)) == EOF)
    {
        return -1;
    }

    line = *lineptr;
    size = *n;

    if (line == NULL)
    {
        line = malloc(128);
        if (line == NULL)
        {
            return -1;
        }
        size = 128;
    }

    read = 0;

    while(c != EOF)
    {
        if (read >= (size - 1))
        {
            size = size + 128;
            rline = realloc(line, size);
            if (rline == NULL)
            {
                free(line);
                *lineptr = NULL;
                *n = 0;
                return -1;
            }
            else
            {
                line = rline;
            }
        }
        line[read++] = c;
        if (c == '\n')
        {
            break;
        }
        c = fgetc(stream);
    }

    line[read] = '\0';
    *lineptr = line;
    *n = size;

    return (long) read;
}

/* Remaining functions are not related to POSIX, but are useful. */

char* os_trim(char *str)
{
    while (isspace((unsigned char)*str))
    {
        str++;
    }

    if (*str != '\0')
    {
        char *edge = str + strlen(str) - 1;
        while ((edge > str) && isspace((unsigned char)*edge))
        {
            *edge = '\0';
            edge--;
        }
    }

    return str;
}

char* os_trim_newline(char *str)
{
    if (*str != '\0')
    {
        char *edge = str + strlen(str) - 1;
        while ((edge > str) && ((*edge == '\n') || (*edge == '\r')))
        {
            *edge = '\0';
            edge--;
        }
        if ((*edge == '\n') || (*edge == '\r'))
        {
            *edge = '\0';
            edge--;
        }
    }

    return str;
}

