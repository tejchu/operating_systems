#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *strdup(const char *s) // stackframe is created, the whole function and automatic
                            // variables get allocated there
{
    char *p = NULL; //stack
    size_t len; //stack
    //text segment - instructions following
    if (s)
    {
        len = strlen(s); //still in stack just value gets updated
        p = malloc(len + 1); //the memory p points to is allocated on the heap
       //text segment - instructions following
        if (p)
        {
            strcpy(p, s);//stack frame and text instructions
        }
    }
    return p;
}


int main()  // stackframe is created, the whole function and automatic
            // variables get allocated there
            // the instructions representing main, written in machine
            // code go in the text segment
{
    static char m[] = "Hello World!";   //static variables get allocated 
                                        // in the data segment
    char *p = strdup(m);    // the pointer is put on the stack
                            // additionally strdup() creates a new stackframe
    
    //text segment all the instructions following
    if (!p) 
    {
        perror("strdup"); //stack frame
        return EXIT_FAILURE;
    }
    if (puts(p) == EOF)
    {
        perror("puts"); //stack frame
        return EXIT_FAILURE;
    }
    if (fflush(stdout) == EOF)
    {
        perror("fflush"); //stack frame
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
