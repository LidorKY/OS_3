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

void hash_file_1(char *filename, char *hash)
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
    FILE *fp;
    char *array = (char *)calloc(SIZE_OF_FILE, sizeof(char));
    size_t bytes_read;
    fp = fopen("sendme.txt", "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: could not open file '%s'\n", "sendme.txt");
        exit(1);
    }
    if (ferror(fp))
    {
        fprintf(stderr, "Error: could not read from file '%s'\n", "sendme.txt");
        exit(1);
    }
    while ((bytes_read = fread(array, 1, SIZE_OF_FILE, fp)) != 0)
    {
    }
    fclose(fp);
    if (send(sock, "start_time", 11, 0) == -1) // send the time we have started to send the file in the socket -"sender_socket"
    {
        perror("error in sending the start time.");
        exit(1);
    }
    ssize_t temp = 0;
    start = clock();
    if ((temp = send(ipv4_tcp_socket, array, SIZE_OF_FILE, 0)) == -1) // sendng the actual file.
    {
        perror("error in sending the file.");
        exit(1);
    }
    end = clock();
    cpu_time_used = (double)(end - start) / (CLOCKS_PER_SEC / 1000);
    printf(",%f\n", cpu_time_used);
    // printf("the size: %zd\n", temp);
    close(ipv4_tcp_socket);
    free(array);
    if (send(sock, "finish_time", 12, 0) == -1) // send the time we have finished to send the file in the socket -"sender_socket"
    {
        perror("error in sending the start time.");
        exit(1);
    }
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

    char *filename = "sendme.txt";
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
    {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    int fd = fileno(fp);
    off_t file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    char buffer[60000];
    int total_sent = 0, bytes_sent;
    if (send(sock, "start_time", 11, 0) == -1) // send the time we have started to send the file in the socket -"sender_socket"
    {
        perror("error in sending the start time.");
        exit(1);
    }
    start = clock();
    while (total_sent < file_size)
    {
        int remaining = file_size - total_sent;
        int to_send = remaining < 60000 ? remaining : 60000;
        if (fread(buffer, 1, to_send, fp) != to_send)
        {
            perror("Error reading file");
            exit(EXIT_FAILURE);
        }
        bytes_sent = sendto(client_socket, buffer, to_send, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (bytes_sent < 0)
        {
            perror("Error sending message");
            exit(EXIT_FAILURE);
        }
        total_sent += bytes_sent;
    }
    end = clock();
    if (send(sock, "finish_time", 12, 0) == -1) // send the time we have finished to send the file in the socket -"sender_socket"
    {
        perror("error in sending the start time.");
        exit(1);
    }
    printf("Sent %ld bytes\n", total_sent);
    cpu_time_used = (double)(end - start) / (CLOCKS_PER_SEC / 1000);
    printf(",%f\n", cpu_time_used);
    fclose(fp);
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

    /*----hashing the file + printing the hash.----*/
    char hash[1000];
    bzero(hash, 1000);
    hash_file_1("sendme.txt", hash); // need to hash here the file. - have already a function for it.
    char hex_hash[33];
    for (int i = 0; i < 32; i++)
    {
        sprintf(&hex_hash[i * 2], "%02x", (unsigned int)hash[i]);
    }
    hex_hash[32] = '\0';
    printf("Hash value: %s\n", hex_hash);
    /*----hashing the file + printing the hash.----*/

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
    printf("%s_%s", TYPE, PARAM);
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