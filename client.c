#include "stdio.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "string.h"
#include "arpa/inet.h"
#include "stdlib.h"
#include "unistd.h"
#include "netinet/in.h"
#include "netinet/tcp.h"


int client()
{
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
    struct sockaddr_in Receiver_address;           // initialize where to send
    Receiver_address.sin_family = AF_INET;         // setting for IPV4
    Receiver_address.sin_port = htons(9999);       // port is 9999
    Receiver_address.sin_addr.s_addr = INADDR_ANY; // listening to all (like 0.0.0.0)
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
    send(sender_socket, "hello", 5, 0);
    char server_response[100];
    recv(sender_socket, &server_response, sizeof(server_response), 0);
    printf("server reponse: %s", server_response);
    close(sender_socket);
    return 0;
}