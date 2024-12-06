#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct kermit_package {
    uint8_t inicio;
    uint16_t info;         // tamanho, sequencia, tipo
    uint8_t dados[64];
    uint8_t crc;
} kermit_t;

int create_raw_socket(char* interface);
