#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

extern char **environ;

// print environment
void print_env()
{
    char **s = environ;
    while (*s != NULL)
    {
        int cr_penv = printf("%s\n", *s);
        // ERROR HANDLING PRINTF
        if (cr_penv != strlen(*s) + 1)
        {
            perror("Error in print_env()");
            exit(EXIT_FAILURE);
        }
        s++;
    }
}

int verbose = 0;
int shouldPrint = 0;

int main(int argc, char *argv[])
{
    int opt, cmdIndex = -1;
    // ENV
    if (argc == 1)
    {
        print_env(STDOUT_FILENO);
        return EXIT_SUCCESS;
    }
    else
    {
        // OPTIONS [ARGS]
        while ((opt = getopt(argc, argv, "vu:")) != -1)
        {
            switch (opt)
            {
            case 'v':
                verbose = 1;
                break;
            case 'u':
                if (unsetenv(optarg) == -1)
                {
                    // ERROR HANDLING UNSETENV
                    perror("Error unsetting variable");
                    exit(EXIT_FAILURE);
                }
                else if (verbose)
                {
                    int cr_pu = fprintf(stderr, "#env unset: %s\n", optarg);
                    // ERROR HANDLING FPRINTF
                    if (cr_pu != strlen(optarg) + 13)
                    {
                        perror("error in print - unsetting");
                    }
                }
                shouldPrint = 1;
                break;
            }
        }
        for (int i = optind; i < argc; i++)
        {
            // PUTENV() -> NAME=VALUE
            if (strchr(argv[i], '=') != NULL)
            {
                int cr_senv = putenv(argv[i]);
                // ERROR HANDLING PUTENV
                if (cr_senv == -1)
                {
                    perror("Error setting variable PUTENV");
                    exit(EXIT_FAILURE);
                }
                if (verbose)
                {
                    int cr_ps = fprintf(stderr, "#env set: %s\n", argv[i]);
                    // ERROR HANDLING FPRINTF
                    if (cr_ps != strlen(argv[i]) + 11)
                    {
                        perror("error in verbose print - PUTENV");
                    }
                }
                shouldPrint = 1;
            }
            else
            {
                cmdIndex = i;
                break;
            }
        }
        // COMMAND [ARGS]
        if (cmdIndex == -1)
        {
            if (verbose && shouldPrint)
            {
                print_env();
            }
            else if (verbose)
            {
                print_env();
            }
        }
        else
        {
            char **newArgv = argv + cmdIndex;
            if (verbose)
            {
                int cr_pexe = fprintf(stderr, "#env executing: %s\n", newArgv[0]);
                // ERROR HANDLING FPRINTF
                if (cr_pexe != strlen(newArgv[0]) + 17)
                {
                    perror("error in verbose print - EXECVP");
                }
            }
            execvp(newArgv[0], newArgv);
            // ERROR HANDLING EXECVP
            printf("Failed executing command %s\n", newArgv[0]);
        }
    }
}
