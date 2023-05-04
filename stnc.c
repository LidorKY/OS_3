#include "stdio.h"
#include <string.h>

int main(int argc, char *argv[])
{
    if ((strcmp(argv[1],"-s") == 0))
    {
        server(argv[2]);
    }
    else if ((strcmp(argv[1],"-c") == 0) )
    {
        client(argv[2], argv[3]);
    }
    else
    {
        printf("\nerror.\n");
    }
    return 0;
}