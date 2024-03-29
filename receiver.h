#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include <stdint.h>

void hash_2(uint8_t *array, size_t array_size,int q_flag);
int ipv4_tcp_receiver(char *IP, char *port, int sock, int q_flag);
int ipv4_udp_receiver(char *IP, char *port, int sock, int q_flag);
int ipv6_tcp_receiver(char *IP, char *port, int sock, int q_flag);
int ipv6_udp_receiver(char *IP, char *port, int sock, int q_flag);
int receiver(char *PORT, int q_flag);