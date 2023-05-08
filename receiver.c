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
#include <openssl/evp.h>
#include "receiver.h"
#define SIZE_OF_FILE 101260000

#include <openssl/evp.h>

void hash_2(uint8_t *array, size_t array_size)
{
    EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;

    // Initialize the MD5 context
    EVP_DigestInit(md_ctx, EVP_md5());

    // Update the hash context with the array data
    EVP_DigestUpdate(md_ctx, array, array_size);

    // Finalize the hash and store the result in the 'hash' buffer
    EVP_DigestFinal(md_ctx, hash, &hash_len);

    // Print the resulting hash
    for (int i = 0; i < hash_len; ++i)
    {
        printf("%02x", hash[i]);
    }
    printf("\n");

    EVP_MD_CTX_free(md_ctx);
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
    uint8_t *buffer = (uint8_t *)calloc(SIZE_OF_FILE, sizeof(uint8_t));
    char timer[20];
    int counter = 0;
    size_t totalReceived = 0;
    size_t remaining = SIZE_OF_FILE;
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
        if (totalReceived == SIZE_OF_FILE && counter >= 2)
        {
            printf("the size: %zu\n", totalReceived);
            hash_2(buffer, SIZE_OF_FILE);
            free(buffer);
            close(client_socket);
            close(receiver_socket);
            return 0;
        }
        else if (pfd[0].revents & POLLIN) // means we got something to read
        {
            bzero(timer, 20);
            read(pfd[0].fd, timer, 20);
            printf("got: %s", timer);
            printf("\n");
            counter++;
        }
        else if (pfd[1].revents & POLLIN)
        {
            uint8_t temp[60000];
            bzero(temp, 60000);
            size_t chunkSize = (remaining < 60000) ? remaining : 60000;
            ssize_t received = recv(client_socket, temp, chunkSize, 0);
            if (received < 0)
            {
                perror("Failed to receive data");
                exit(1);
            }
            memcpy(buffer + totalReceived, temp, received);
            totalReceived += received;
            remaining -= received;
        }
    }
    hash_2(buffer, SIZE_OF_FILE);
    free(buffer);
    close(client_socket);
    close(receiver_socket);
    return 0;
}

int ipv4_udp_receiver(char *IP, char *port, int sock)
{
    struct pollfd pfd[2];
    int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }
    // int client_addr_len = sizeof(server_addr);
    // size_t current_size = 0;
    int n;
    pfd[0].fd = sock; // from input;
    pfd[0].events = POLLIN;
    pfd[0].revents = 0;
    pfd[1].fd = server_socket; // from socket;
    pfd[1].events = POLLIN;
    pfd[1].revents = 0;
    char timer[20];
    uint8_t *buffer = (uint8_t *)calloc(SIZE_OF_FILE, sizeof(uint8_t));
    int counter = 0;
    // ssize_t received = 0;
    size_t totalReceived = 0;
    size_t remaining = SIZE_OF_FILE;
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
        if (totalReceived == SIZE_OF_FILE && counter >= 2)
        {
            printf("the size: %zu\n", totalReceived);
            hash_2(buffer, SIZE_OF_FILE);
            free(buffer);
            close(server_socket);
            return 0;
        }
        else if (pfd[0].revents & POLLIN) // means we got something to read
        {
            bzero(timer, 20);
            read(pfd[0].fd, timer, 20);
            printf("got: %s", timer);
            printf("\n");
            counter++;
        }
        else if (pfd[1].revents & POLLIN)
        {
            uint8_t temp[1500];
            bzero(temp, 1500);
            socklen_t serverAddrLen = sizeof(server_addr);
            ssize_t received = recvfrom(server_socket, temp, sizeof(temp), 0, (struct sockaddr *)&server_addr, &serverAddrLen);
            if (received < 0)
            {
                perror("Failed to receive data");
                exit(1);
            }
            memcpy(buffer + totalReceived, temp, received);
            totalReceived += received;
            remaining -= received;
        }
    }

    hash_2(buffer, SIZE_OF_FILE);
    free(buffer);
    close(server_socket);
    return 0;
}

int ipv6_tcp_receiver(char *IP, char *port, int sock)
{
    struct pollfd pfd[2];
    int receiver_socket;
    receiver_socket = socket(AF_INET6, SOCK_STREAM, 0);
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
    struct sockaddr_in6 Sender_address, new_addr;
    Sender_address.sin6_family = AF_INET6;
    Sender_address.sin6_port = htons(atoi(port));
    Sender_address.sin6_addr = in6addr_any;
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
    uint8_t *buffer = (uint8_t *)calloc(SIZE_OF_FILE, sizeof(uint8_t));
    char timer[20];
    int counter = 0;
    size_t totalReceived = 0;
    size_t remaining = SIZE_OF_FILE;
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
        if (totalReceived == SIZE_OF_FILE && counter >= 2)
        {
            printf("the size: %zu\n", totalReceived);
            hash_2(buffer, SIZE_OF_FILE);
            free(buffer);
            close(client_socket);
            close(receiver_socket);
            return 0;
        }
        else if (pfd[0].revents & POLLIN) // means we got something to read
        {
            bzero(timer, 20);
            read(pfd[0].fd, timer, 20);
            sleep(0.5);//without this sleep it does'nt work.
            printf("got: %s", timer);
            printf("\n");
            counter++;
        }
        else if (pfd[1].revents & POLLIN)
        {
            uint8_t temp[60000];
            bzero(temp, 60000);
            size_t chunkSize = (remaining < 60000) ? remaining : 60000;
            ssize_t received = recv(client_socket, temp, chunkSize, 0);
            if (received < 0)
            {
                perror("Failed to receive data");
                exit(1);
            }
            memcpy(buffer + totalReceived, temp, received);
            totalReceived += received;
            remaining -= received;
        }
    }

    hash_2(buffer, SIZE_OF_FILE);
    free(buffer);
    close(client_socket);
    return 0;
}

int ipv6_udp_receiver(char *IP, char *port, int sock)
{
    struct pollfd pfd[2];
    int server_socket = socket(AF_INET6, SOCK_DGRAM, 0);
    if (server_socket < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(atoi(port));
    inet_pton(AF_INET6, IP, &(server_addr.sin6_addr));
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }
    // int client_addr_len = sizeof(server_addr);
    // size_t current_size = 0;
    int n;
    pfd[0].fd = sock; // from input;
    pfd[0].events = POLLIN;
    pfd[0].revents = 0;
    pfd[1].fd = server_socket; // from socket;
    pfd[1].events = POLLIN;
    pfd[1].revents = 0;
    char timer[20];
    uint8_t *buffer = (uint8_t *)calloc(SIZE_OF_FILE, sizeof(uint8_t));
    int counter = 0;
    // ssize_t received = 0;
    size_t totalReceived = 0;
    size_t remaining = SIZE_OF_FILE;
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
        if (totalReceived == SIZE_OF_FILE && counter >= 2)
        {
            printf("the size: %zu\n", totalReceived);
            hash_2(buffer, SIZE_OF_FILE);
            free(buffer);
            close(server_socket);
            return 0;
        }
        else if (pfd[0].revents & POLLIN) // means we got something to read
        {
            bzero(timer, 20);
            read(pfd[0].fd, timer, 20);
            printf("got: %s", timer);
            printf("\n");
            counter++;
        }
        else if (pfd[1].revents & POLLIN)
        {
            uint8_t temp[1500];
            bzero(temp, 1500);
            socklen_t serverAddrLen = sizeof(server_addr);
            ssize_t received = recvfrom(server_socket, temp, sizeof(temp), 0, (struct sockaddr *)&server_addr, &serverAddrLen);
            if (received < 0)
            {
                perror("Failed to receive data");
                exit(1);
            }
            memcpy(buffer + totalReceived, temp, received);
            totalReceived += received;
            remaining -= received;
        }
    }

    hash_2(buffer, SIZE_OF_FILE);
    free(buffer);
    close(server_socket);
    return 0;
}

int receiver(char *PORT)
{

    // struct pollfd pfd[2];

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

    if (strcmp(TYPE, "ipv4") == 0 && strcmp(PARAM, "tcp") == 0)
    {
        ipv4_tcp_receiver(IP, port, client_socket);
    }
    else if (strcmp(TYPE, "ipv4") == 0 && strcmp(PARAM, "udp") == 0)
    {
        ipv4_udp_receiver(IP, port, client_socket);
    }
    else if (strcmp(TYPE, "ipv6") == 0 && strcmp(PARAM, "tcp") == 0)
    {
        ipv6_tcp_receiver(IP, port, client_socket);
    }
    else if (strcmp(TYPE, "ipv6") == 0 && strcmp(PARAM, "udp") == 0)
    {
        ipv6_udp_receiver(IP, port, client_socket);
    }

    // receive the initial time in socket - "client_socket".
    // receiving the file in the secondry socket - must to use poll here - only because arkady said so.
    // receiving finish time
    // printing to the terminal the things arkady mentioned.

    close(client_socket);

    return 0;
}