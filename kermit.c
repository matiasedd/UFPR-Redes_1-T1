#include "kermit.h"

int create_raw_socket(char* interface) {
    // Cria arquivo para o socket sem qualquer protocolo
    int soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (soquete == -1) {
        fprintf(stderr, "Erro ao criar socket: Verifique se você é root!\n");
        exit(-1);
    }
 
    int ifindex = if_nametoindex(interface);
 
    struct sockaddr_ll endereco = {0};
    endereco.sll_family = AF_PACKET;
    endereco.sll_protocol = htons(ETH_P_ALL);
    endereco.sll_ifindex = ifindex;
    // Inicializa socket
    if (bind(soquete, (struct sockaddr*) &endereco, sizeof(endereco)) == -1) {
        fprintf(stderr, "Erro ao fazer bind no socket\n");
        exit(-1);
    }
 
    struct packet_mreq mr = {0};
    mr.mr_ifindex = ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    // Não joga fora o que identifica como lixo: Modo promíscuo
    if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
        fprintf(stderr, "Erro ao fazer setsockopt: "
            "Verifique se a interface de rede foi especificada corretamente.\n");
        exit(-1);
    }
 
    return soquete;
}

uint8_t get_tipo(kermit_t *pacote) {
    return (pacote->info >> 11) & OFFSET_5;
}

void imprime_pacote(kermit_t *pacote)
{
    uint8_t tamanho = pacote->info & OFFSET_6;          // Bits 0-5
    uint8_t sequencia = (pacote->info >> 6) & OFFSET_5; // Bits 6-10
    uint8_t tipo = (pacote->info >> 11) & OFFSET_5;     // Bits 11-15

    printf("%08b %06b %05b %05b %08b\n", pacote->inicio, tamanho, sequencia, tipo, pacote->crc);
}

void montar_pacote(uint8_t tipo, kermit_t *pacote, char *dados, uint8_t tamanho, uint8_t sequencia)
{
    memset(pacote, 0, sizeof(kermit_t));

    pacote->inicio = INICIO;
    pacote->info = (tamanho & OFFSET_6) | ((sequencia & OFFSET_5) << 6) | ((tipo & OFFSET_5) << 11);
    memcpy(pacote->dados, dados, tamanho);
    pacote->crc = 0x0;
}
