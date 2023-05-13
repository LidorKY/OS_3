#include "stdio.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "sys/stat.h"
#include "string.h"
#include "arpa/inet.h"
#include "stdlib.h"
#include "unistd.h"
#include "netinet/in.h"
#include "netinet/tcp.h"
#include <time.h>
#include <poll.h>

int server(char *PORT)
{

    struct pollfd pfd[2];

    // creating a socket
    int receiver_socket;
    receiver_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (receiver_socket == -1)
    {
        printf("-there is a problem with initializing receiver\n");
    }
    else
    {
        printf("-initialize successfully.\n");
    }

    int optval = 1;

    if (setsockopt(receiver_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
    {
        perror("setsockoopt");
        exit(1);
    }
    //--------------------------------------------------------------------------------
    // initialize where to send
    struct sockaddr_in Sender_address, new_addr;
    Sender_address.sin_family = AF_INET;
    Sender_address.sin_port = htons(atoi(PORT));
    Sender_address.sin_addr.s_addr = INADDR_ANY;
    //---------------------------------------------------------------------------------
    // connecting the Receiver and Sender
    int bindd = bind(receiver_socket, (struct sockaddr *)&Sender_address, sizeof(Sender_address));
    if (bindd == -1)
    {
        printf("-there is a problem with bindding.\n");
    }
    else
    {
        printf("-bindding successfully.\n");
    }
    //---------------------------------------------------------------------------------

    int sock_queue = listen(receiver_socket, 1); // now it can listen to two senders in pareral.
    if (sock_queue == -1)
    { // if there are already 2 senders.
        printf("-queue is full, can't listen.\n");
    }
    else
    {
        printf("-listening...\n");
    }
    // initialize the socket for comunicating with the Sender.
    int client_socket; // the socket
    socklen_t addr_size = sizeof(new_addr);
    client_socket = accept(receiver_socket, (struct sockaddr *)&new_addr, &addr_size); // the func return socket discriptor of a new
    // socket and information of the Sender like IP and Port into new_addr.
    //---------------------------------------------------------------------------------

    int n;

    pfd[0].fd = 0; // from input;
    pfd[0].events = POLLIN;
    pfd[0].revents = 0;

    pfd[1].fd = client_socket; // from socket;
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
            send(client_socket, buffer, 1024, 0);
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
    close(client_socket);
    return 0;
}