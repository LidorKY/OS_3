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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#define SIZE_OF_FILE 101260000

void hash_file_2(char *filename, char *hash)
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

int ipv4_tcp_receiver(char *IP, char *port, int sock)
{
    struct pollfd pfd[2];
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
    Sender_address.sin_port = htons(atoi(port));
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
    pfd[0].fd = sock; // from input;
    pfd[0].events = POLLIN;
    pfd[0].revents = 0;
    pfd[1].fd = client_socket; // from socket;
    pfd[1].events = POLLIN;
    pfd[1].revents = 0;
    FILE *fp;
    fp = fopen("gotme.txt", "wb+");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: could not open file '%s'\n", "gotme.txt");
        exit(1);
    }
    if (ferror(fp))
    {
        printf("here\n");
        fprintf(stderr, "Error: could not read from file '%s'\n", "gotme.txt");
        exit(1);
    }
    char buffer[100000];
    char timer[20];
    int counter = 0;
    size_t current_size = 0;
    while (1)
    {
        n = poll(pfd, 2, 5000);
        if (n < 0)
        {
            printf("error on poll\n");
            continue;
        }
        if (n == 0)
        {
            printf("timeout...\n");
            break;
        }

        for (int i = 0; i < 2; i++)
        {
            if (pfd[i].revents & POLLIN) // means we got something to read
            {
                if (pfd[i].fd == sock && counter < 2)
                {
                    bzero(timer, 20);
                    read(pfd[0].fd, timer, 20);
                    printf("got: %s", timer);
                    printf("\n");
                    counter++;
                }
                else if (pfd[i].fd == client_socket && current_size < SIZE_OF_FILE)
                {
                    bzero(buffer, 100000);
                    current_size += read(pfd[1].fd, buffer, 100000);
                    fprintf(fp, "%s", buffer);
                }
                else if (current_size >= SIZE_OF_FILE && counter >= 2)
                {
                    printf("the size: %zu\n", current_size);
                    fclose(fp);
                    FILE *f = fopen("gotme.txt", "rwb");
                    if (f == NULL)
                    {
                        return -1;
                    }
                    size_t to_delete = 0;
                    if (fseek(f, 0, SEEK_END) != 0)
                    {
                        fclose(f);
                        return -1;
                    }
                    to_delete = ftell(f);
                    fclose(f);

                    FILE *p = fopen("gotme.txt", "rb+");
                    if (p == NULL)
                    {
                        return -1;
                    }
                    if (fseek(p, -(to_delete - SIZE_OF_FILE), SEEK_END) != 0)
                    {
                        perror("fseek");
                        exit(1);
                    }
                    long pos = ftell(p);
                    if (pos == -1)
                    {
                        perror("ftell");
                        exit(1);
                    }
                    if (ftruncate(fileno(p), pos) != 0)
                    {
                        perror("ftruncate");
                        exit(1);
                    }
                    if (fclose(p) != 0)
                    {
                        perror("fclose");
                        exit(1);
                    }
                    /*----hashing the file + printing the hash.----*/
                    char hash[1000];
                    bzero(hash, 1000);
                    hash_file_2("gotme.txt", hash); // need to hash here the file. - have already a function for it.
                    char hex_hash[33];
                    for (int i = 0; i < 32; i++)
                    {
                        sprintf(&hex_hash[i * 2], "%02x", (unsigned int)hash[i]);
                    }
                    hex_hash[32] = '\0';
                    printf("Hash value: %s\n", hex_hash);
                    /*----hashing the file + printing the hash.----*/
                    close(client_socket);
                    close(receiver_socket);
                    return 0;
                }
            }
        }
    }
    close(fp);
    close(client_socket);
    close(receiver_socket);
    return 0;
}

int ipv4_udp_receiver() { return 0; }
int ipv6_tcp_receiver() { return 0; }
int ipv6_udp_receiver() { return 0; }

int receiver(char *PORT)
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
    char IP[16];
    bzero(IP, 16);
    char port[6];
    bzero(port, 6);
    char TYPE[5];
    bzero(TYPE, 5);
    char PARAM[9];
    bzero(PARAM, 9);
    recv(client_socket, IP, 16, 0);
    printf("the ip is: %s", IP);
    printf("\n");
    sleep(1);
    recv(client_socket, port, 6, 0);
    printf("the port is: %s", port);
    printf("\n");
    sleep(1);
    recv(client_socket, TYPE, 5, 0);
    printf("the type is: %s", TYPE);
    printf("\n");
    sleep(1);
    recv(client_socket, PARAM, 9, 0);
    printf("the param is: %s", PARAM);
    printf("\n");

    // receive 1 type of 8 different communcations and open a correct socket.
    ipv4_tcp_receiver(IP, port, client_socket);
    // int ipv4_udp_receiver();
    // int ipv6_tcp_receiver();
    // int ipv6_udp_receiver();

    // receive the initial time in socket - "client_socket".
    // receiving the file in the secondry socket - must to use poll here - only because arkady said so.
    // receiving finish time
    // printing to the terminal the things arkady mentioned.

    close(client_socket);

    return 0;
}