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
#define SIZE_OF_FILE 101260000

void hash_file(char *filename, char *hash)
{
    FILE *fp;
    char *array = (char *)calloc(SIZE_OF_FILE, sizeof(char));
    size_t bytes_read;
    MD5_CTX context;

    // Open the file in binary mode
    fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: could not open file '%s'\n", filename);
        exit(1);
    }

    // Check if the file was opened successfully
    if (ferror(fp))
    {
        fprintf(stderr, "Error: could not read from file '%s'\n", filename);
        exit(1);
    }

    // Initialize the MD5 context
    MD5_Init(&context);

    // Read the file in chunks and update the hash context
    while ((bytes_read = fread(array, 1, SIZE_OF_FILE, fp)) != 0)
    {
        MD5_Update(&context, array, bytes_read);
    }

    // Finalize the hash and store the result in the provided buffer
    MD5_Final((unsigned char *)hash, &context);
    free(array);

    // Close the file
    fclose(fp);
}

int sender(char *IP, char *PORT)
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

    char hash[1000];
    bzero(hash, 1000);
    hash_file("sendme.txt", hash); // need to hash here the file. - have already a function for it.
    char hex_hash[33];
    for (int i = 0; i < 32; i++)
    {
        sprintf(&hex_hash[i * 2], "%02x", (unsigned int)hash[i]);
    }
    printf("here\n");
    hex_hash[32] = '\0';

    printf("Hash value: %s\n", hex_hash);

    // need to create here 8 different comumunications.
    // firstly send in the socket -"sender_socket" to the receiver which socket he needs to open.
    // then, hash the file.
    // send the time we have started to send the file in the socket -"sender_socket"
    // send the file
    // send the time we have finished to send the file in the socket -"sender_socket"
    // dont need to use poll here just loop of sending the file.

    close(sender_socket);
    return 0;
}