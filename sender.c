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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>

int client(char *IP, char *PORT)
{
    struct pollfd pfd[1];

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
    Receiver_address.sin_port = htons(atoi(PORT));       // port is 9999
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



    //need to hash here the file. - have already a function for it.
    //need to create here 8 different comumunications.
    //firstly send in the socket -"sender_socket" to the receiver which socket he needs to open.
    //then, hash the file.
    //send the time we have started to send the file in the socket -"sender_socket"
    //send the file
    //send the time we have finished to send the file in the socket -"sender_socket"
    //dont need to use poll here just loop of sending the file.





    close(sender_socket);
    return 0;
}