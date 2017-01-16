
#include "clique_enum.h"
#include "clique_tomita.h"
#include "benchmark.h"
#include "platform.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

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
    if (argc < 2)
    {
        printf("Enumerates cliques by Tomita et al. algorithm.\n");
        printf("\n");
        printf("Usage:\n");
        printf("  %s <input-dimacs-files>...\n", "tomita");
        printf("  %s --\n", "tomita");
        printf("\n");
        printf("  --  take file names from standard input\n");
        printf("\n");
    }
    else
    {
        size_t index = 1;

        bool write_headers = false;
        bool process_stdin = false;

        FILE* fout;
        clique_context_t cc;
        clique_context_tomita(&cc);

        fout = stdout;

        if (write_headers)
        {
            fprintf(fout, "algorithm,filename,num-vertices,num-edges,deg-variance,max-clique-size,num-rec-calls,num-in-pivot,num-search-restarts,num-cliques,seconds\n");
            fflush(fout);
        }

        for (; index < (size_t)argc; index++)
        {
            uint64_t numCliques;
            double seconds;

            if (strcmp(argv[index], "--") == 0)
            {
                process_stdin = true;
                break;
            }
            else
            if (file_exists(argv[index]))
            {
                clique_enum_test(fout, argv[index], &cc, &numCliques, &seconds);
            }
            else
            {
                printf(" %.2d. %s: file not found! ", (int)(index), argv[index]);
            }
            fflush(fout);
        }

        if (process_stdin)
        {
            uint64_t numCliques;
            double seconds;
            char *line = NULL;
            size_t len = 0;

            while ((os_getline(&line, &len, stdin)) != -1)
            {
                char* filename = os_trim_newline(line);

                if (file_exists(filename))
                {
                    clique_enum_test(fout, filename, &cc, &numCliques, &seconds);
                }
                else
                if (strcmp(filename, "") == 0)
                {
                    break;
                }
                else
                {
                    printf(" %.2d. %s: file not found! \n", (int)(index), filename);
                }
                fflush(fout);
                index++;
            }

            free(line);
        }

        fclose(fout);

        clique_context_destroy(&cc);
        return 0;
    }
}
