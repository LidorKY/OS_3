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
#include <time.h>
#define SIZE_OF_FILE 101260000

uint8_t *generate()
{
    uint8_t *array = (uint8_t *)calloc(SIZE_OF_FILE, sizeof(uint8_t));
    if (array == NULL)
    {
        return -1;
    }
    srand(time(NULL));
    for (uint32_t i = 0; i < SIZE_OF_FILE; i++)
    {
        *(array + i) = ((uint32_t)rand() % 256);
    }
    return array;
}

void hash_1(uint8_t *array, size_t array_size)
{
    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5_CTX context;

    // Initialize the MD5 context
    MD5_Init(&context);

    // Update the hash context with the array data
    MD5_Update(&context, array, array_size);

    // Finalize the hash and store the result in the 'hash' buffer
    MD5_Final(hash, &context);

    // Print the resulting hash
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
    {
        printf("%02x", hash[i]);
    }
    printf("\n");
}

int ipv4_tcp_sender(char *IP, char *PORT, int sock)
{
    clock_t start, end;
    double cpu_time_used;
    sleep(3);
    int ipv4_tcp_socket;
    ipv4_tcp_socket = socket(AF_INET, SOCK_STREAM, 0); // because we are in linux the default cc is cubic.
    if (ipv4_tcp_socket == -1)
    {
        printf("there is a problem with initializing sender.\n");
    }
    else
    {
        // printf("-initialize successfully.\n");
    }
    //--------------------------------------------------------------------------------
    // initialize where to send
    struct sockaddr_in Receiver_address;              // initialize where to send
    Receiver_address.sin_family = AF_INET;            // setting for IPV4
    Receiver_address.sin_port = htons(atoi(PORT));    // port is 9999
    Receiver_address.sin_addr.s_addr = inet_addr(IP); // listening to all (like 0.0.0.0)
    //---------------------------------------------------------------------------------
    // connecting the Sender and Receiver
    int connection_status = connect(ipv4_tcp_socket, (struct sockaddr *)&Receiver_address, sizeof(Receiver_address));
    if (connection_status == -1)
    {
        printf("there is an error with the connection.\n");
    }
    else
    {
        // printf("-connected.\n");
    }
    //---------------------------------------------------------------------------------
    // send the file
    uint8_t *sendme = generate(); // need to add hash here -> currently located in the main function.
    hash_1(sendme, SIZE_OF_FILE);
    if (send(sock, "start_time", 11, 0) == -1) // send the time we have started to send the file in the socket -"sender_socket"
    {
        perror("error in sending the start time.");
        exit(1);
    }
    ssize_t temp = 0;
    size_t totalSent = 0;
    size_t remaining = SIZE_OF_FILE;
    start = clock();
    while (remaining > 0)
    {
        size_t chunkSize = (remaining < 60000) ? remaining : 60000;
        ssize_t sent = send(ipv4_tcp_socket, sendme + totalSent, chunkSize, 0);
        if (sent < 0)
        {
            perror("Failed to send data");
            exit(1);
        }
        totalSent += sent;
        remaining -= sent;
        // printf("Sent %zu bytes (%.2f%%)\n", totalSent, (float)totalSent / SIZE_OF_FILE * 100);
    }
    end = clock();
    cpu_time_used = (double)(end - start) / (CLOCKS_PER_SEC / 1000);
    printf(",%f\n", cpu_time_used);
    printf("the size: %zd\n", totalSent);
    close(ipv4_tcp_socket);
    free(sendme);
    if (send(sock, "finish_time", 12, 0) == -1) // send the time we have finished to send the file in the socket -"sender_socket"
    {
        perror("error in sending the start time.");
        exit(1);
    }
    close(sock);
    return 0;
}

int ipv4_udp_sender(char *IP, char *PORT, int sock)
{
    clock_t start, end;
    double cpu_time_used;
    sleep(3);
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(PORT));
    server_addr.sin_addr.s_addr = inet_addr(IP);

    uint8_t *sendme = generate(); // need to add hash here -> currently located in the main function.
    hash_1(sendme, SIZE_OF_FILE);

    if (send(sock, "start_time", 11, 0) == -1) // send the time we have started to send the file in the socket -"sender_socket"
    {
        perror("error in sending the start time.");
        exit(1);
    }
    size_t bytes_sent = 0;
    size_t remaining_bytes = SIZE_OF_FILE;
    start = clock();
    while (remaining_bytes > 0)
    {
        size_t chunk_size = (remaining_bytes > 60000) ? 60000 : remaining_bytes;
        ssize_t num_bytes_sent = sendto(client_socket, sendme, chunk_size, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (num_bytes_sent < 0)
        {
            perror("Error sending data");
            exit(1);
        }
        bytes_sent += num_bytes_sent;
        remaining_bytes -= num_bytes_sent;
        sendme += num_bytes_sent;
    }
    end = clock();
    if (send(sock, "finish_time", 12, 0) == -1) // send the time we have finished to send the file in the socket -"sender_socket"
    {
        perror("error in sending the start time.");
        exit(1);
    }
    printf("Sent %zu bytes\n", bytes_sent);
    cpu_time_used = (double)(end - start) / (CLOCKS_PER_SEC / 1000);
    printf(",%f\n", cpu_time_used);
    sleep(10);
    close(client_socket);
    return 0;
}

int ipv6_tcp_sender(char *IP, char *PORT, int sock) { return 0; }
int ipv6_udp_sender(char *IP, char *PORT, int sock) { return 0; }

int sender(char *IP, char *PORT, char *TYPE, char *PARAM)
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
    struct sockaddr_in Receiver_address;                       // initialize where to send
    Receiver_address.sin_family = AF_INET;                     // setting for IPV4
    Receiver_address.sin_port = htons(9999);                   // port is 9999
    Receiver_address.sin_addr.s_addr = inet_addr("127.0.0.1"); // listening to all (like 0.0.0.0)
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

    // firstly send in the socket -"sender_socket" to the receiver which socket he needs to open.
    send(sender_socket, IP, 16, 0);
    // printf("the ip is: %s", IP);
    // printf("\n");
    sleep(1);
    send(sender_socket, PORT, 6, 0);
    // printf("the port is: %s", PORT);
    // printf("\n");
    sleep(1);
    send(sender_socket, TYPE, 5, 0);
    // printf("the type is: %s", TYPE);
    // printf("\n");
    sleep(1);
    send(sender_socket, PARAM, 9, 0);
    // printf("the param is: %s", PARAM);
    // printf("\n");
    printf("%s_%s\n", TYPE, PARAM);
    // printf("\n");

    // need to create here 8 different comumunications.
    if (strcmp(TYPE, "ipv4") == 0 && strcmp(PARAM, "tcp") == 0)
    {
        ipv4_tcp_sender(IP, PORT, sender_socket);
    }
    else if (strcmp(TYPE, "ipv4") == 0 && strcmp(PARAM, "udp") == 0)
    {
        ipv4_udp_sender(IP, PORT, sender_socket);
    }
    else if (strcmp(TYPE, "ipv6") == 0 && strcmp(PARAM, "tcp") == 0)
    {
        ipv6_tcp_sender(IP, PORT, sender_socket);
    }
    else if (strcmp(TYPE, "ipv6") == 0 && strcmp(PARAM, "udp") == 0)
    {
        ipv6_udp_sender(IP, PORT, sender_socket);
    }
    close(sender_socket);
    return 0;
}