#include "kermit.h"

void montar_pacote(kermit_t *pacote, const char *mensagem, uint8_t sequencia, uint8_t tipo) {
    memset(pacote, 0, sizeof(kermit_t));

    pacote->inicio = 0b01111110;
    uint8_t tamanho = strlen(mensagem);
    if (tamanho > 64) tamanho = 64;

    pacote->info = (tamanho & 0b00111111) | ((sequencia & 0b00011111) << 6) | ((tipo & 0b00011111) << 11);

    memcpy(pacote->dados, mensagem, tamanho);

    pacote->crc = 0x0;
}

void imprime_pacote(kermit_t *pacote)
{
    // Desmonta o campo info
    uint8_t tamanho = pacote->info & 0b00111111;          // Bits 0-5
    uint8_t sequencia = (pacote->info >> 6) & 0b00011111; // Bits 6-10
    uint8_t tipo = (pacote->info >> 11) & 0b00011111;     // Bits 11-15

    // Exibe informações do pacote
    printf("\tInicio: 0b%08b\n", pacote->inicio);
    printf("\tInfo: Tamanho=%d, Sequência=%d, Tipo=0b%05b\n", tamanho, sequencia, tipo);
    printf("\tDados: %.*s\n", tamanho, pacote->dados);
    printf("\tCRC: 0b%08b\n", pacote->crc);
}

int server_backup(kermit_t *recebido, int sockfd, struct sockaddr_ll destino)
{
    printf("Solicitação de backup recebida. Processando...\n");

    FILE *arquivo = fopen((char *)recebido->dados, "w");

    if (arquivo == NULL)
    {
        perror("Erro ao salvar arquivo");
        return -1;
    }

    char nome_arq[64];
    kermit_t enviado;
    montar_pacote(&enviado, nome_arq, 1, 0b00000);
    
    // Envio do pacote
    ssize_t bytes_enviados = sendto(sockfd, &enviado, sizeof(kermit_t), 0,
                                    (struct sockaddr*)&destino, sizeof(destino));
    if (bytes_enviados == -1) {
        perror("Erro ao enviar pacote");
        return -1;
    }

    printf("Pacote enviado com sucesso (%ld bytes)\n", bytes_enviados);

    return 0;
}

int main()
{
    const char *interface = "enp0s31f6"; // Interface para escutar
    int sockfd = create_raw_socket((char *)interface);

    struct sockaddr_ll destino = {0};
    destino.sll_family = AF_PACKET;
    destino.sll_ifindex = if_nametoindex(interface);
    destino.sll_protocol = ETH_P_ALL;

    puts("Servidor inicializado: aguardando o envio de pacotes");

    while (1)
    {
        char buffer[2048];
        struct sockaddr_ll remetente;
        socklen_t remetente_len = sizeof(remetente);

        // Recebe o pacote
        ssize_t bytes_recebidos = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                                           (struct sockaddr *)&remetente, &remetente_len);
        if (bytes_recebidos == -1)
        {
            perror("Erro ao receber pacote");
            continue;
        }

        // Valida se o pacote é do tipo kermit_t
        if (bytes_recebidos < sizeof(kermit_t))
        {
            printf("Pacote ignorado (tamanho insuficiente: %ld bytes)\n", bytes_recebidos);
            continue;
        }

        // Processa o pacote
        kermit_t *pacote = (kermit_t *)buffer;
        if (pacote->inicio == 0b01111110)
        { // Verifica campo de início
            imprime_pacote(pacote);

            uint8_t tipo = (pacote->info >> 11) & 0b00011111;     // Bits 11-15

            // Ações baseadas no tipo
            switch (tipo)
            {
            case 0b00100: // backup
                server_backup(pacote, sockfd, destino);
                break;
            case 0b10000: // Dados
                printf("Recebendo dados para backup...\n");
                break;
            case 0b10001: // Fim da transmissão
                printf("Transmissão de dados finalizada.\n");
                break;
            default:
                printf("Tipo de pacote não reconhecido: 0b%05b\n", tipo);
            }
        }
        else
        {
            printf("Pacote ignorado (campo de início inválido: 0b%08b)\n", pacote->inicio);
        }
    }

    return 0;
}
