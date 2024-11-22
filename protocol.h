#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <signal.h>
#include <net/if.h>

#define MAX_DATA_SIZE 63
#define INPUT_BUFFER_SIZE 1024
#define DEVICE "enp0s31f6"

typedef struct 
{
    uint8_t mark;
    uint16_t field;
    uint8_t data[MAX_DATA_SIZE];
    uint8_t crc;
} kermit_package_t;

/* --- Client --- */

void client_backup(int sockfd, struct sockaddr_ll *server_addr, const char *pathname);

void restaura();

void verifica();

/* --- Server --- */

