#include <stdlib.h>
#include <stdio.h>  /* perror, printf */
#include <string.h> /* memset */

#include <unistd.h> /* close */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <arpa/inet.h> /* htons */
#include <net/if.h>

#include <math.h> /* log10 */

typedef struct kermit_package
{
	uint8_t inicio;     /* marcador de inicio */
	uint16_t info;      /* tamanho, sequencia, tipo */
	uint8_t dados[63];  /* dados */
	uint8_t crc;        /* crc de 8 bits calculado sobre os campos inicio, info e dados */
} kermit_t;

#define NET_INTERFACE "lo"

#define INICIO   0b01111110
#define OFFSET_5 0b00011111
#define OFFSET_6 0b00111111

#define ACK      0b00000
#define NACK     0b00001
#define OK       0b00010

#define BACKUP   0b00100
#define RESTAURA 0b00101
#define VERIFICA 0b00110

#define TAMANHO	 0b01111
#define DADOS    0b10000
#define FINALIZA 0b10001
#define ERRO     0b11111

int create_raw_socket(char *interface);  /* Retorna um descritor de arquivo com o socket */

uint16_t get_tamanho(kermit_t *pacote);   /* Retorna o tamanho do pacote */

uint16_t get_tipo(kermit_t *pacote);      /* Retorna o tipo do pacote */

uint16_t get_sequencia(kermit_t *pacote); /* Retorna o numero de sequencia do pacote */

void imprime_pacote(kermit_t *pacote);   /* Imrpime: inicio tamanho sequencia tipo crc */

void montar_pacote(uint16_t tipo, kermit_t *pacote, char *dados, uint16_t tamanho, uint16_t sequencia); /* atribui os dados ao pacote*/

void enviar_pacote(kermit_t *pacote, int sockfd);

void receber_pacote(kermit_t *pacote, int sockfd);
