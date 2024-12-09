#include "kermit.h"

/* --- Socket Manipulation --- */

int create_raw_socket(char *interface)
{
    /* --- Handle Socket --- */

#if DEBUG
    puts("[create_raw_socket]: Iniciando Socket");
#endif

    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (sockfd < 0)
    {
        perror("[create_raw_socket]: Erro ao criar socket: Verifique se você é root!\n");
        exit(-1);
    }

    struct ifreq ir; /* Configuracoes da interface de redes */

    memset(&ir, 0, sizeof(struct ifreq));
    memcpy(ir.ifr_name, interface, strlen(interface) * sizeof(char));

    if (ioctl(sockfd, SIOCGIFINDEX, &ir) < 0)
    {
        perror("[create_raw_socket]: Erro syscall ioctl");
        exit(-2);
    }

    struct sockaddr_ll server_addr; /* device-independent physical layer address */

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sll_family = AF_PACKET;
    server_addr.sll_protocol = htons(ETH_P_ALL);
    server_addr.sll_ifindex = ir.ifr_ifindex;

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("[create_raw_socket]: Erro no bind");
        exit(-3);
    }

    struct packet_mreq mr;

    memset(&mr, 0, sizeof(mr));
    mr.mr_ifindex = ir.ifr_ifindex;
    mr.mr_type = PACKET_MR_PROMISC;

    if (setsockopt(sockfd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) < 0)
    {
        perror("[create_raw_socket]: Erro ao fazer setsockopt: Verifique se a interface de rede foi especificada corretamente.");
        exit(-4);
    }

    struct timeval timeout;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;

    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout)) < 0) {
        perror("[create_raw_socket]: Erro ao adicionar timeout");
        exit(-5);
    }

#if DEBUG
    puts("[create_raw_socket]: Socket Iniciado com Sucesso");
#endif

    return sockfd;
}

/* --- Kermit_package --- */

uint16_t get_tamanho(kermit_t *pacote)
{
    uint16_t aux = pacote->info;
    aux = aux >> 10;

    return aux;
}

uint16_t get_sequencia(kermit_t *pacote)
{
    uint16_t volatile aux = pacote->info;
    aux = aux << 6;
    aux = aux >> 11;

    return aux;
}

uint16_t get_tipo(kermit_t *pacote)
{
    uint16_t volatile aux = pacote->info;
    aux = aux << 11;
    aux = aux >> 11;

    return aux;
}

uint8_t get_crc(kermit_t *pacote)
{
    return pacote->crc;
}

void imprime_pacote(kermit_t *pacote)
{
    printf("[imprime_pacote]: [INIT] %hhu [TAM] %hhu [SEQ] %hhu [TIP] 0x%x [CRC] %hhu dados: %s\n", 
            pacote->inicio, get_tamanho(pacote), get_sequencia(pacote), get_tipo(pacote), get_crc(pacote), pacote->dados);
}

uint8_t calcular_crc(uint8_t *bytes, uint16_t size)
{
#define POLY 0x97 /*10010111 - CRC8-AUTOSAR */
    uint8_t crc = 0x00;

    while (--size)
    {
        crc ^= *bytes++; /* Apply Byte */

        for (uint8_t i = 0; i < 8; ++i) /* Para cada Bit */
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

    pacote->inicio = INICIO;
    pacote->info = pacote->info | ((tamanho << 10) );
    pacote->info = pacote->info | ((sequencia << 5) );
    pacote->info = pacote->info | (tipo);

    memcpy(pacote->dados, dados, tamanho);
    pacote->crc = calcular_crc((uint8_t *) pacote, tamanho + 3);
}

void enviar_pacote(kermit_t *pacote, int sockfd)
{
    size_t bytes = send(sockfd, pacote, sizeof(kermit_t), 0);

#if DEBUG
    printf("[enviar_pacote]: send (%ld): ", bytes);
    imprime_pacote(pacote);
#endif
}

long long timestamp() {
    struct timeval timer;
    gettimeofday(&timer, NULL);

    return timer.tv_sec;
}

/* receber_pacote
 *
 *  0 - Recebeu um pacote valido do procotolo
 * -1 - Timeou 
 */

int receber_pacote(kermit_t *pacote, int sockfd)
{
    memset(pacote, 0, sizeof(kermit_t));

    size_t bytes;

    long long start = timestamp();
    long long deadline = start + TIMEOUT;
    
    do
    {
        bytes = recv(sockfd, pacote, sizeof (kermit_t), 0);

        if (pacote->inicio == INICIO)  /* foi recebido um pacote valido antes do timeout */
        {
            bytes = recv(sockfd, pacote, sizeof (kermit_t), 0); /* remover loopback */
#if DEBUG
            // puts("[receber_pacote]: Recebi um pacote valido");
            printf("[receber_pacote]: recv (%ld): ", bytes);
            imprime_pacote(pacote);
#endif
            return 0; /* pacote valido */
        }
#if DEBUG
        else
        {
            //if( bytes != -1)
            //  puts("[receber_pacote]: Recebi lixo");
        }
#endif

    } while (timestamp() < deadline);

#if DEBUG
    //puts("[receber_pacote]: Timeout"); 

#endif

    return -1; /* timeout */
}
