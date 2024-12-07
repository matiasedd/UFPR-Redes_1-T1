#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct kermit_package
{
	uint8_t inicio;
	uint16_t info;			/* tamanho, sequencia, tipo */
	uint8_t dados[64];
	uint8_t crc;
} kermit_t;

#define NET_INTERFACE "lo"

#define INICIO 0b01111110
#define OFFSET_5 0b00011111
#define OFFSET_6 0b00111111

#define ACK 0b00000
#define NACK 0b00001
#define OK 0b00010

#define BACKUP 0b00100
#define RESTAURA 0b00101
#define VERIFICA 0b00110

#define DADOS 0b10000
#define FINALIZA 0b10001
#define ERRO 0b11111

int create_raw_socket(char *interface);

uint8_t get_tipo(kermit_t *pacote);

void imprime_pacote(kermit_t *pacote);

void montar_pacote(uint8_t tipo, kermit_t *pacote, char *dados, uint8_t tamanho, uint8_t sequencia);
