#include "kermit.h"

/* --- Socket Manipulation --- */

int create_raw_socket(char* interface) {
    /* --- Handle Socket --- */

    #if DEBUG
        puts("Iniciando Socket");
    #endif 

    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (sockfd < 0) {
        perror("Erro ao criar socket: Verifique se você é root!\n");
        return -1;
    }

    struct ifreq ir; /* Configuracoes da interface de redes */

    memset(&ir, 0, sizeof (struct ifreq));
    memcpy(ir.ifr_name, interface, strlen(interface) * sizeof (char));

    if (ioctl(sockfd, SIOCGIFINDEX, &ir) < 0) {
        perror("Erro syscall ioctl");
        return -2;
    }

    struct sockaddr_ll server_addr; /* device-independent physical layer address */

    memset(&server_addr, 0, sizeof (server_addr));
    server_addr.sll_family = AF_PACKET;
    server_addr.sll_protocol = htons(ETH_P_ALL);
    server_addr.sll_ifindex = ir.ifr_ifindex;

    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof (server_addr)) < 0) {
        perror("Erro no bind");
        return -3;
    }

    struct packet_mreq mr;

    memset(&mr, 0, sizeof (mr));
    mr.mr_ifindex = ir.ifr_ifindex;
    mr.mr_type = PACKET_MR_PROMISC;

    if (setsockopt(sockfd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) < 0) {
        perror("Erro ao fazer setsockopt: Verifique se a interface de rede foi especificada corretamente.");
        return -4;
    }

    #if DEBUG
        puts("Socket Iniciado com Sucesso");
    #endif
 
    return sockfd;
}

/* --- Kermit_package --- */

uint8_t get_tamanho(kermit_t *pacote) {
    return pacote->info & OFFSET_6;
}

uint8_t get_sequencia(kermit_t *pacote) {
    return  (pacote->info >> 6) & OFFSET_5;
}

uint8_t get_tipo(kermit_t *pacote) {
    return (pacote->info >> 11) & OFFSET_5;
}


void imprime_pacote(kermit_t *pacote)
{
    uint8_t tamanho = pacote->info & OFFSET_6;          // Bits 0-5
    uint8_t sequencia = (pacote->info >> 6) & OFFSET_5; // Bits 6-10
    uint8_t tipo = (pacote->info >> 11) & OFFSET_5;     // Bits 11-15

    printf("[INIT] %08b [TAM] %06b [SEQ] %05b [TYPE] %05b [CRC] %08b\n", pacote->inicio, tamanho, sequencia, tipo, pacote->crc);
}

void montar_pacote(uint8_t tipo, kermit_t *pacote, char *dados, uint8_t tamanho, uint8_t sequencia)
{
    memset(pacote, 0, sizeof(kermit_t));

    pacote->inicio = INICIO;
    pacote->info = (tamanho & OFFSET_6) | ((sequencia & OFFSET_5) << 6) | ((tipo & OFFSET_5) << 11);
    memcpy(pacote->dados, dados, tamanho);
    pacote->crc = 0x0;
}
