#include "stdio.h"
#include <string.h>

int main(int argc, char *argv[])
{
    if ((strcmp(argv[1], "-s") == 0))
    {
        server(argv[2]);
    }
    else if ((strcmp(argv[1], "-c") == 0) && (strcmp(argv[4], "-p") == 0) && (argv[5] != NULL) && (argv[6] != NULL)) //add type and param
    {
        sender(argv[2], argv[3], argv[5], argv[6]);
    }
    else if ((strcmp(argv[1], "-c") == 0))
    {
        client(argv[2], argv[3]);
    }
    else if ((strcmp(argv[1], "receiver") == 0))
    {
        receiver(argv[2]);
    }
    else
    {
        printf("\nerror.\n");
    }
    return 0;
}