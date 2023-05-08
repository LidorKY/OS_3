#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include <stdint.h>

uint8_t *generate();
void hash_1(uint8_t *array, size_t array_size);
int ipv4_tcp_sender(char *IP, char *PORT, int sock);
int ipv4_udp_sender(char *IP, char *PORT, int sock);
int ipv6_tcp_sender(char *IP, char *PORT, int sock);
int ipv6_udp_sender(char *IP, char *PORT, int sock);
int sender(char *IP, char *PORT, char *TYPE, char *PARAM);



