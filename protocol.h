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

typedef struct 
{
    uint8_t mark;
    uint8_t size;
    uint8_t seq;
    uint8_t type;
    uint8_t data[MAX_DATA_SIZE];
    uint8_t crc;
} kermit_package_t;
