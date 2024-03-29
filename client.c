#include "stdio.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "string.h"
#include "arpa/inet.h"
#include "stdlib.h"
#include "unistd.h"
#include "netinet/in.h"
#include "netinet/tcp.h"
#include <poll.h>

int client(char *IP, char *PORT)
{
    struct pollfd pfd[2];

    // creating a socket
    int sender_socket;
    sender_socket = socket(AF_INET, SOCK_STREAM, 0); // because we are in linux the default cc is cubic.
    if (sender_socket == -1)
    {
        printf("there is a problem with initializing sender.\n");
    }
    else
    {
        printf("-initialize successfully.\n");
    }
    //--------------------------------------------------------------------------------
    // initialize where to send
    struct sockaddr_in Receiver_address;              // initialize where to send
    Receiver_address.sin_family = AF_INET;            // setting for IPV4
    Receiver_address.sin_port = htons(atoi(PORT));    // port is 9999
    Receiver_address.sin_addr.s_addr = inet_addr(IP); // listening to all (like 0.0.0.0)
    //---------------------------------------------------------------------------------
    // connecting the Sender and Receiver
    int connection_status = connect(sender_socket, (struct sockaddr *)&Receiver_address, sizeof(Receiver_address));
    if (connection_status == -1)
    {
        printf("there is an error with the connection.\n");
    }
    else
    {
        printf("-connected.\n");
    }
    //---------------------------------------------------------------------------------

    int n;

    pfd[0].fd = 0; // from input;
    pfd[0].events = POLLIN;
    pfd[0].revents = 0;

    pfd[1].fd = sender_socket; // from socket;
    pfd[1].events = POLLIN;
    pfd[1].revents = 0;

    char buffer[1024];
    while (1)
    {
        // communication with client
        n = poll(pfd, 2, -1);

        if (n < 0)
        {
            printf("error on poll\n");
            continue;
        }
        if (n == 0)
        {
            printf("waiting...\n");
            continue;
        }

        if (pfd[0].revents & POLLIN)
        {
            // read from input and send to client socket
            bzero(buffer, 1024);
            read(pfd[0].fd, buffer, 1024);
            bzero(stdin, 1024);
            send(sender_socket, buffer, 1024, 0);
        }
        else if (pfd[1].revents & POLLIN)
        {
            // read from client socket and print to console
            bzero(buffer, 1024);
            read(pfd[1].fd, buffer, 1024);
            bzero(stdin, 1024);
            printf("%s", buffer);
        }
    }
    close(sender_socket);
    return 0;
}