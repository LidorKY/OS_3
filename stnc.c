#include "stdio.h"


int main(int argc, char *argv[])
{
    printf(argv[1]);
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