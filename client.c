#include "kermit.h"

void montar_pacote(kermit_t *pacote, const char *mensagem, uint8_t sequencia, uint8_t tipo)
{
    memset(pacote, 0, sizeof(kermit_t));

    pacote->inicio = 0b01111110;
    uint8_t tamanho = strlen(mensagem);
    if (tamanho > 64)
        tamanho = 64;

    pacote->info = (tamanho & 0b00111111) | ((sequencia & 0b00011111) << 6) | ((tipo & 0b00011111) << 11);

    memcpy(pacote->dados, mensagem, tamanho);

    pacote->crc = 0x0;
}

void processar_pacote(kermit_t *pacote)
{
    // Desmonta o campo info
    uint8_t tamanho = pacote->info & 0b00111111;          // Bits 0-5
    uint8_t sequencia = (pacote->info >> 6) & 0b00011111; // Bits 6-10
    uint8_t tipo = (pacote->info >> 11) & 0b00011111;     // Bits 11-15


    switch(tipo) {
        case 0b00000:
            puts("Received OK from server");
            break;
        default:
            // Exibe informações do pacote
            printf("Pacote recebido:\n");
            printf("Inicio: 0b%08b\n", pacote->inicio);
            printf("Info: Tamanho=%d, Sequência=%d, Tipo=0b%05b\n", tamanho, sequencia, tipo);
            printf("Dados: %.*s\n", tamanho, pacote->dados);
            printf("CRC: 0b%08b\n", pacote->crc);
    }
}

int main()
{
    const char *interface = "enp0s31f6";
    int sockfd = create_raw_socket((char *)interface);

    struct sockaddr_ll destino = {0};
    destino.sll_family = AF_PACKET;
    destino.sll_ifindex = if_nametoindex(interface);
    destino.sll_halen = ETH_ALEN;
    memset(destino.sll_addr, 0xFF, ETH_ALEN); // Broadcast MAC address

    char nome_arq[64];

    fgets(nome_arq, 64, stdin);
    nome_arq[strlen(nome_arq) - 1] = '\0';

    /* BACKUP */
    kermit_t pacote;

    FILE *arquivo = fopen(nome_arq, "r");

    if (arquivo == NULL)
    {
        perror("Erro ao abrir arquivo");
        return -1;
    }

    montar_pacote(&pacote, nome_arq, 1, 0b00100);

    // Envio do pacote
    ssize_t bytes_enviados = sendto(sockfd, &pacote, sizeof(kermit_t), 0,
                                    (struct sockaddr *)&destino, sizeof(destino));
    if (bytes_enviados == -1)
    {
        perror("Erro ao enviar pacote");
        return -1;
    }

    printf("Pacote enviado com sucesso (%ld bytes)\n", bytes_enviados);

    char buffer[2048];
    struct sockaddr_ll remetente;
    socklen_t remetente_len = sizeof(remetente);
    ssize_t bytes_recebidos = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                                       (struct sockaddr *)&remetente, &remetente_len);
    if (bytes_recebidos == -1)
    {
        perror("Erro ao receber pacote");
        // continue;
    }

    // Valida se o pacote é do tipo kermit_t
    if (bytes_recebidos < sizeof(kermit_t))
    {
        printf("Pacote ignorado (tamanho insuficiente: %ld bytes)\n", bytes_recebidos);
        // continue;
    }

    // Processa o pacote
    kermit_t *pkg = (kermit_t *)buffer;
    if (pkg->inicio == 0b01111110)
    { // Verifica campo de início
        processar_pacote(pkg);
    }
    else
    {
        printf("Pacote ignorado (campo de início inválido: 0b%08b)\n", pkg->inicio);
    }

    fclose(arquivo);

    return 0;
}
