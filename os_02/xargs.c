#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void printTrace(char **arr, int index)
{
    // printing on stderr
    for (int i = 0; i < index; i++)
    {
        fprintf(stderr, "%s ", arr[i]);
    }
    fprintf(stderr, "\n");
}
void parentWait(pid_t pid)
{
    int allowedWait = 16, wait = 0;
    while (wait < allowedWait)
    {
        if ((allowedWait = waitpid(pid, NULL, 0)) != -1)
        {
            break;
        }
        wait++;
    }
    if (wait == allowedWait)
    {
        perror("waitpid exceed maximum waiting times");
        exit(EXIT_FAILURE);
    }
}
int main(int argc, char *argv[])
{
    int opt;
    int n = -1;
    int trace = 0;
    char *myargv[1024];
    ssize_t len;
    size_t cap;
    char *text;
    int count = 0;
    int cmdIndex = 0;
    int restIndex;

    while ((opt = getopt(argc, argv, "tn:")) != -1)
    {
        switch (opt)
        {
        case 't':
            trace = 1;
            break;
        case 'n':
            n = atoi(optarg);
            break;
        }
    }
    if (argc != optind)
    {
        for (int i = optind; i < argc; i++)
        {
            myargv[cmdIndex] = argv[i];
            cmdIndex++;
        }
    }
    else
    {
        myargv[0] = "/bin/echo";
        cmdIndex = 1;
    }
    restIndex = cmdIndex;

    while ((len = getline(&text, &cap, stdin)) != -1)
    {
        if (text[len - 1] == '\n')
        {
            len--;
        }
        myargv[restIndex] = strndup(text, len);
        if (myargv[restIndex] == NULL)
        {
            perror("strndup() failed");
            exit(EXIT_FAILURE);
        }
        restIndex++;
        count++;
        if (count == n || restIndex == 1024)
        {
            myargv[restIndex] = '\0';
            int rc = fork();
            if (!rc)
            {
                if (trace)
                {
                    printTrace(myargv, restIndex);
                }
                execvp(myargv[0], myargv);
                perror("execvp failed");
                return EXIT_FAILURE;
            }
            if (rc == -1)
            {
                perror("fork failed");
                return EXIT_FAILURE;
            }
            parentWait(rc);
            count = 0;
            restIndex = cmdIndex;
        }
    }
    if (restIndex != cmdIndex)
    {
        myargv[restIndex] = '\0';
        int rc = fork();
        if (!rc)
        {
            if (trace)
            {
                printTrace(myargv, restIndex);
            }
            execvp(myargv[0], myargv);
            perror("execvp failed");
            return EXIT_FAILURE;
        }
        if (rc == -1)
        {
            perror("fork failed");
            return EXIT_FAILURE;
        }
        parentWait(rc);
    }
    return EXIT_SUCCESS;
}