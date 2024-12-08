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

uint8_t get_tamanho(kermit_t *pacote) 
{
    return (pacote->info >> 10);
}

uint8_t get_sequencia(kermit_t *pacote) 
{
    return  (pacote->info << 6) >> 11;
}

uint8_t get_tipo(kermit_t *pacote) 
{
    return (pacote->info << 11) >> 11;
}


void imprime_pacote(kermit_t *pacote)
{
    uint8_t tamanho = get_tamanho(pacote);     // Bits 0-5
    uint8_t sequencia = get_sequencia(pacote); // Bits 6-10
    uint8_t tipo = get_tipo(pacote);           // Bits 11-15

    printf("[INIT] %08b [INFO] %hhu [CRC] %08b\n", pacote->inicio, pacote->info, pacote->crc);
}

uint8_t calcular_crc(uint8_t *bytes, uint16_t size) 
{
    #define POLY 0x97                       /*10010111 - CRC8-AUTOSAR */
    uint8_t crc = 0x00;

    while (--size)
    {
        crc ^= *bytes++;                    /* Apply Byte */

        for (uint8_t i = 0; i < 8; ++i)     /* Para cada Bit */
        {
            crc <<= 1;

            if (crc & 0x80)                 /* 10000000 */
                crc ^=  POLY;
        }

    }

    return crc;
}


void montar_pacote(uint16_t tipo, kermit_t *pacote, char *dados, uint16_t tamanho, uint16_t sequencia)
{
    memset(pacote, 0, sizeof(kermit_t));

    printf("debug: %hhu %hhu %hhu\n", tamanho, sequencia, tipo);
    pacote->inicio = INICIO;
    pacote->info = ((tamanho << 10) | (tipo));
    // memcpy(pacote->dados, dados, tamanho);
    // pacote->crc = calcular_crc((uint8_t *) pacote, tamanho + 3);

    //#if DEBUG
    //  printf("(Tipo:%hhu Tamanho:%hhu Sequencia:%hhu\n", tipo, tamanho, sequencia);
        // printf("Pacote: "); imprime_pacote(pacote);
    //#endif
}

