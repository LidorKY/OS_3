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

int server()
{
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
    //--------------------------------------------------------------------------------
    // initialize where to send
    struct sockaddr_in Sender_address, new_addr;
    Sender_address.sin_family = AF_INET;
    Sender_address.sin_port = htons(9999);
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

    int sock_queue = listen(receiver_socket, 2); // now it can listen to two senders in pareral.
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
    char client_message[100];
    recv(client_socket, &client_message, sizeof(client_message), 0);
    printf("server reponse: %s", client_message);
    send(client_socket, "how are you doing", 50, 0);
    close(client_socket);

    return 0;
}