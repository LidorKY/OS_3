#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <openssl/md5.h>
#include <time.h>
#include <openssl/evp.h>
#include "sender.h"
#define SIZE_OF_FILE 105260000

#define UDS_PATH "/tmp/uds_socket" // Replace with your desired UDS socket path

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/mman.h>


char* ipv4_to_ipv6(const char* ipv4) {
    char* ipv6 = (char*) malloc(INET6_ADDRSTRLEN * sizeof(char));
    if (ipv6 == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    // Convert the IPv4 address to binary form
    struct in_addr ipv4_addr;
    if (inet_pton(AF_INET, ipv4, &ipv4_addr) != 1) {
        perror("Failed to convert IPv4 address");
        exit(EXIT_FAILURE);
    }

    // Convert the IPv4 address to IPv4-mapped IPv6 address format
    struct in6_addr ipv6_addr;
    memset(&ipv6_addr, 0, sizeof(ipv6_addr));
    ipv6_addr.s6_addr[10] = 0xff;
    ipv6_addr.s6_addr[11] = 0xff;
    memcpy(&ipv6_addr.s6_addr[12], &ipv4_addr, sizeof(ipv4_addr));
    
    // Convert the IPv6 address to string form
    if (inet_ntop(AF_INET6, &ipv6_addr, ipv6, INET6_ADDRSTRLEN) == NULL) {
        perror("Failed to convert IPv6 address");
        exit(EXIT_FAILURE);
    }

    return ipv6;
}

uint8_t *generate()
{
    uint8_t *array = (uint8_t *)calloc(SIZE_OF_FILE, sizeof(uint8_t));
    if (array == NULL)
    {
        return NULL;
    }
    srand(time(NULL));
    for (uint32_t i = 0; i < SIZE_OF_FILE; i++)
    {
        *(array + i) = ((uint32_t)rand() % 256);
    }
    return array;
}

void hash_1(uint8_t *array, size_t array_size, int q_flag)
{
    unsigned char hash[MD5_DIGEST_LENGTH];
    EVP_MD_CTX *context = EVP_MD_CTX_new();

    // Initialize the MD5 context
#ifdef _OPENSSL_API_COMPAT
    EVP_MD_CTX_init(context);
#else
    EVP_DigestInit_ex(context, EVP_md5(), NULL);
#endif

    // Update the hash context with the array data
    EVP_DigestUpdate(context, array, array_size);

    // Finalize the hash and store the result in the 'hash' buffer
#ifdef _OPENSSL_API_COMPAT
    EVP_MD_CTX_finish(context, hash);
#else
    EVP_DigestFinal_ex(context, hash, NULL);
#endif

    // Print the resulting hash
    if(!q_flag){
        for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
        {
            printf("%02x", hash[i]);
        }
        printf("\n");
    }

    EVP_MD_CTX_free(context);
}

int ipv4_tcp_sender(char *IP, char *PORT, int sock,int q_flag)
{
    sleep(3);
    int ipv4_tcp_socket;
    ipv4_tcp_socket = socket(AF_INET, SOCK_STREAM, 0); // because we are in linux the default cc is cubic.
    if (ipv4_tcp_socket == -1)
    {
        // printf("there is a problem with initializing sender.\n");
    }
    else
    {
        // printf("-initialize successfully.\n");
    }
    //--------------------------------------------------------------------------------
    // initialize where to send
    struct sockaddr_in Receiver_address;               // initialize where to send
    Receiver_address.sin_family = AF_INET;             // setting for IPV4
    Receiver_address.sin_port = htons(atoi(PORT) + 3); // port is 9999
    Receiver_address.sin_addr.s_addr = inet_addr(IP);  // listening to all (like 0.0.0.0)
    //---------------------------------------------------------------------------------
    // connecting the Sender and Receiver
    int connection_status = connect(ipv4_tcp_socket, (struct sockaddr *)&Receiver_address, sizeof(Receiver_address));
    if (connection_status == -1)
    {
        // printf("there is an error with the connection.\n");
    }
    else
    {
        // printf("-connected.\n");
    }
    //---------------------------------------------------------------------------------
    // send the file
    uint8_t *sendme = generate(); // need to add hash here -> currently located in the main function.
    hash_1(sendme, SIZE_OF_FILE,q_flag);
    if (send(sock, "start_time", 11, 0) == -1) // send the time we have started to send the file in the socket -"sender_socket"
    {
        perror("error in sending the start time.");
        exit(1);
    }
    // ssize_t temp = 0;
    size_t totalSent = 0;
    size_t remaining = SIZE_OF_FILE;
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
    }
    free(sendme);
    if (send(sock, "finish_time", 12, 0) == -1) // send the time we have finished to send the file in the socket -"sender_socket"
    {
        perror("error in sending the start time.");
        exit(1);
    }
    sleep(5);
    close(ipv4_tcp_socket);
    close(sock);
    return 0;
}

int ipv4_udp_sender(char *IP, char *PORT, int sock,int q_flag)
{
    // clock_t start, end;
    // double cpu_time_used;
    sleep(1);
    int client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(PORT) + 3);
    server_addr.sin_addr.s_addr = inet_addr(IP);

    uint8_t *sendme = generate(); // need to add hash here -> currently located in the main function.
    hash_1(sendme, SIZE_OF_FILE,q_flag);

    if (send(sock, "start_time", 11, 0) == -1) // send the time we have started to send the file in the socket -"sender_socket"
    {
        perror("error in sending the start time.");
        exit(1);
    }
    // ssize_t temp = 0;
    size_t totalSent = 0;
    size_t remaining = SIZE_OF_FILE;
    // start = clock();
    while (remaining > 0)
    {
        size_t chunkSize = (remaining < 1500) ? remaining : 1500;
        ssize_t sent = sendto(client_socket, sendme + totalSent, chunkSize, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (sent < 0)
        {
            perror("Failed to send data");
            exit(1);
        }
        totalSent += sent;
        remaining -= sent;
    }
    // end = clock();
    if (send(sock, "finish_time", 12, 0) == -1) // send the time we have finished to send the file in the socket -"sender_socket"
    {
        perror("error in sending the start time.");
        exit(1);
    }
    // printf("Sent %zu bytes\n", totalSent);
    // cpu_time_used = (double)(end - start) / (CLOCKS_PER_SEC / 1000);
    // printf(",%f\n", cpu_time_used);
    sleep(7);
    close(client_socket);
    return 0;
}

int ipv6_tcp_sender(char *IP, char *PORT, int sock, int q_flag)
{
    int ipv6_tcp_socket;
    ipv6_tcp_socket = socket(AF_INET6, SOCK_STREAM, 0); // using IPV6
    if (ipv6_tcp_socket == -1)
    {
        printf("there is a problem with initializing sender.\n");
    }
    else
    {
        // printf("-initialize successfully.\n");
    }
    //--------------------------------------------------------------------------------
    // initialize where to send
    struct sockaddr_in6 Receiver_address;             // initialize where to send
    Receiver_address.sin6_family = AF_INET6;          // setting for IPV6
    Receiver_address.sin6_port = htons(atoi(PORT) + 4);// port is 9999
    inet_pton(AF_INET6, ipv4_to_ipv6(IP), &Receiver_address.sin6_addr); // IPv6 address of the receiver
    //---------------------------------------------------------------------------------
    // connecting the Sender and Receiver
    sleep(5);
    int connection_status = connect(ipv6_tcp_socket, (struct sockaddr *)&Receiver_address, sizeof(Receiver_address));
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
    hash_1(sendme, SIZE_OF_FILE,q_flag);
    if (send(sock, "start_time", 11, 0) == -1) // send the time we have started to send the file in the socket -"sender_socket"
    {
        perror("error in sending the start time.");
        exit(1);
    }
    // ssize_t temp = 0;
    size_t totalSent = 0;
    size_t remaining = SIZE_OF_FILE;
    while (remaining > 0)
    {
        size_t chunkSize = (remaining < 60000) ? remaining : 60000;
        ssize_t sent = send(ipv6_tcp_socket, sendme + totalSent, chunkSize, 0);
        if (sent < 0)
        {
            perror("Failed to send data");
            exit(1);
        }
        totalSent += sent;
        remaining -= sent;
    }
    free(sendme);
    if (send(sock, "finish_time", 12, 0) == -1) // send the time we have finished to send the file in the socket -"sender_socket"
    {
        perror("error in sending the start time.");
        exit(1);
    }
    sleep(5);
    close(ipv6_tcp_socket);
    close(sock);
    return 0;
}

int ipv6_udp_sender(char *IP, char *PORT, int sock,int q_flag) // noder without gpt
{
    // clock_t start, end;
    // double cpu_time_used;
    sleep(1);
    int client_socket = socket(AF_INET6, SOCK_DGRAM, 0);
    if (client_socket < 0)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin6_family = AF_INET;
    server_addr.sin6_port = htons(atoi(PORT) + 3);
    inet_pton(AF_INET6, IP, &server_addr.sin6_addr);

    uint8_t *sendme = generate(); // need to add hash here -> currently located in the main function.
    hash_1(sendme, SIZE_OF_FILE,q_flag);

    if (send(sock, "start_time", 11, 0) == -1) // send the time we have started to send the file in the socket -"sender_socket"
    {
        perror("error in sending the start time.");
        exit(1);
    }
    // ssize_t temp = 0;
    size_t totalSent = 0;
    size_t remaining = SIZE_OF_FILE;
    // start = clock();
    while (remaining > 0)
    {
        size_t chunkSize = (remaining < 1500) ? remaining : 1500;
        ssize_t sent = sendto(client_socket, sendme + totalSent, chunkSize, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (sent < 0)
        {
            perror("Failed to send data");
            exit(1);
        }
        totalSent += sent;
        remaining -= sent;
    }
    // end = clock();
    if (send(sock, "finish_time", 12, 0) == -1) // send the time we have finished to send the file in the socket -"sender_socket"
    {
        perror("error in sending the start time.");
        exit(1);
    }
    // printf("Sent %zu bytes\n", totalSent);
    // cpu_time_used = (double)(end - start) / (CLOCKS_PER_SEC / 1000);
    // printf(",%f\n", cpu_time_used);
    sleep(7);
    close(client_socket);
    return 0;
}

int uds_stream_sender(int sock,int q_flag)
{
    // clock_t start, end;
    // double cpu_time_used;
    sleep(3);
    int uds_socket;
    uds_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (uds_socket == -1)
    {
        printf("There is a problem with initializing sender.\n");
    }
    else
    {
        // printf("-initialize successfully.\n");
    }
    //--------------------------------------------------------------------------------
    // Initialize where to send
    struct sockaddr_un Receiver_address;
    Receiver_address.sun_family = AF_UNIX;
    strcpy(Receiver_address.sun_path, UDS_PATH);
    //---------------------------------------------------------------------------------
    // Connecting the Sender and Receiver
    int connection_status = connect(uds_socket, (struct sockaddr *)&Receiver_address, sizeof(Receiver_address));
    if (connection_status == -1)
    {
        printf("There is an error with the connection.\n");
    }
    else
    {
        // printf("-connected.\n");
    }
    //---------------------------------------------------------------------------------
    // Send the file
    uint8_t *sendme = generate(); // Need to add hash here - currently located in the main function.
    hash_1(sendme, SIZE_OF_FILE,q_flag);
    if (send(sock, "start_time", 11, 0) == -1)
    {
        perror("Error in sending the start time.");
        exit(1);
    }
    size_t totalSent = 0;
    size_t remaining = SIZE_OF_FILE;
    // start = clock();
    while (remaining > 0)
    {
        size_t chunkSize = (remaining < 60000) ? remaining : 60000;
        ssize_t sent = send(uds_socket, sendme + totalSent, chunkSize, 0);
        if (sent < 0)
        {
            perror("Failed to send data");
            exit(1);
        }
        totalSent += sent;
        remaining -= sent;
    }
    // end = clock();
    // cpu_time_used = (double)(end - start) / (CLOCKS_PER_SEC / 1000);
    // printf(",%f\n", cpu_time_used);
    close(uds_socket);
    // printf("The size: %zd\n", totalSent);
    free(sendme);
    if (send(sock, "finish_time", 12, 0) == -1)
    {
        perror("Error in sending the finish time.");
        exit(1);
    }
    sleep(3);
    close(sock);
    return 0;
}

int uds_dgram_sender(int sock,int q_flag)
{
    // clock_t start, end;
    // double cpu_time_used;
    // sleep(1);
    int uds_socket;
    uds_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (uds_socket == -1)
    {
        printf("There is a problem with initializing sender.\n");
        exit(1);
    }
    //--------------------------------------------------------------------------------
    // Initialize where to send
    struct sockaddr_un Receiver_address;
    Receiver_address.sun_family = AF_UNIX;
    strcpy(Receiver_address.sun_path, UDS_PATH);
    //---------------------------------------------------------------------------------
    // Send the file
    uint8_t *sendme = generate(); // Need to add hash here - currently located in the main function.
    hash_1(sendme, SIZE_OF_FILE,q_flag);
    if (send(sock, "start_time", 11, 0) == -1)
    {
        perror("Error in sending the start time.");
        exit(1);
    }
    size_t totalSent = 0;
    size_t remaining = SIZE_OF_FILE;
    // start = clock();
    while (remaining > 0)
    {
        size_t chunkSize = (remaining < 1500) ? remaining : 1500;
        ssize_t sent = sendto(uds_socket, sendme + totalSent, chunkSize, 0, (struct sockaddr *)&Receiver_address, sizeof(Receiver_address));
        if (sent < 0)
        {
            perror("Failed to send data");
            exit(1);
        }
        totalSent += sent;
        remaining -= sent;
    }
    // end = clock();
    // close(uds_socket);
    // cpu_time_used = (double)(end - start) / (CLOCKS_PER_SEC / 1000);
    // printf(",%f\n", cpu_time_used);
    // printf("The size: %zd\n", totalSent);
    free(sendme);
    if (send(sock, "finish_time", 12, 0) == -1)
    {
        perror("Error in sending the finish time.");
        exit(1);
    }
    sleep(3);
    close(sock);
    return 0;
}

int pipe_sender(char *filename, int sock,int q_flag)
{
    // clock_t start, end;
    // double cpu_time_used;
    // sleep(1);
    int fd;
    size_t totalSent = 0;
    uint8_t *sendme = generate(); // Need to add hash here - currently located in the main function.
    hash_1(sendme, SIZE_OF_FILE,q_flag);

    // Create the FIFO (named pipe) if it doesn't exist
    mknod(filename, __S_IFIFO | 0666, 0);

    // Open the FIFO for writing
    fd = open(filename, O_WRONLY);
    if (fd == -1)
    {
        perror("Error opening FIFO");
        exit(1);
    }
    if (send(sock, "start_time", 11, 0) == -1)
    {
        perror("Error in sending the start time.");
        exit(1);
    }
    // start = clock();
    // Write the data array to the FIFO
    if ((totalSent = write(fd, sendme, SIZE_OF_FILE)) == -1)
    {
        perror("Error writing to FIFO");
        exit(1);
    }
    // end = clock();
    close(fd);
    // cpu_time_used = (double)(end - start) / (CLOCKS_PER_SEC / 1000);
    // printf(",%f\n", cpu_time_used);
    // printf("The size: %zd\n", totalSent);
    if (send(sock, "finish_time", 12, 0) == -1)
    {
        perror("Error in sending the finish time.");
        exit(1);
    }
    sleep(3);
    close(sock);
    // Close the FIFO
    free(sendme);
    return 0;
}

int mmap_sender(char *file_name, int sock,int q_flag)
{
    int fd;
    char *mapped_file;
    char *data;
    int i;

    // open the file for writing
    fd = open(file_name, O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (fd < 0)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // allocate memory for the data
    data = (char *)malloc(SIZE_OF_FILE);
    if (data == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // write random data to the file
    for (i = 0; i < SIZE_OF_FILE; i++)
    {
        data[i] = rand() % 256;
    }

    if (write(fd, data, SIZE_OF_FILE) != SIZE_OF_FILE)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }
    if (send(sock, "start_time", 11, 0) == -1)
    {
        perror("Error in sending the start time.");
        exit(1);
    }
    // map the memory to the process address space
    mapped_file = mmap(NULL, SIZE_OF_FILE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped_file == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // write 100MB of data to the mapped file
    uint8_t *arr = generate();
    memcpy(mapped_file, arr, SIZE_OF_FILE);

    if (send(sock, "finish_time", 12, 0) == -1)
    {
        perror("Error in sending the finish time.");
        exit(1);
    }

    // printf("arr= %s\n",arr);
    hash_1(arr, SIZE_OF_FILE,q_flag);

    // close the file and socket
    close(fd);
    // sleep(8);
    close(sock);
    free(data);
    free(arr);

    return 0;
}

int sender(char *IP, char *PORT, char *TYPE, char *PARAM, int q_flag)
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
    int connection_status = connect(sender_socket, (struct sockaddr *)&Receiver_address, sizeof(Receiver_address));
    if (connection_status == -1)
    {
        printf("there is an error with the connection.\n");
    }
    else
    {
        // printf("-connected.\n");
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
        ipv4_tcp_sender(IP, PORT, sender_socket,q_flag);
    }
    else if (strcmp(TYPE, "ipv4") == 0 && strcmp(PARAM, "udp") == 0)
    {
        ipv4_udp_sender(IP, PORT, sender_socket,q_flag);
    }
    else if (strcmp(TYPE, "ipv6") == 0 && strcmp(PARAM, "tcp") == 0)
    {
        ipv6_tcp_sender(IP, PORT, sender_socket,q_flag);
    }
    else if (strcmp(TYPE, "ipv6") == 0 && strcmp(PARAM, "udp") == 0)
    {
        ipv6_udp_sender(IP, PORT, sender_socket,q_flag);
    }
    else if (strcmp(TYPE, "uds") == 0 && strcmp(PARAM, "stream") == 0)
    {
        uds_stream_sender(sender_socket,q_flag);
    }
    else if (strcmp(TYPE, "uds") == 0 && strcmp(PARAM, "dgram") == 0)
    {
        uds_dgram_sender(sender_socket,q_flag);
    }
    else if (strcmp(TYPE, "pipe") == 0)
    {
        pipe_sender(PARAM, sender_socket,q_flag);
    }
    else if (strcmp(TYPE, "mmap") == 0)
    {
        mmap_sender(PARAM, sender_socket,q_flag);
    }
    close(sender_socket);
    return 0;
}