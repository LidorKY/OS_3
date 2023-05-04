#include "stdio.h"
#include <string.h>

int main(int argc, char *argv[])
{
    if (strcmp(argv[1],"server") == 0)
    {
        server();
    }
    else if (strcmp(argv[1],"client") == 0)
    {
        client();
    }
    else
    {
        printf("\nerror.\n");
    }
    return 0;
}