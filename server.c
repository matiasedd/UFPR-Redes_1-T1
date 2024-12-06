#include "kermit.h"

int server_backup(kermit_t *pacote) {
    printf("Solicitação de backup recebida. Processando...\n");

    FILE *arquivo = fopen((char *) pacote->dados, "w");

    if (arquivo == NULL) {
        perror("Erro ao salvar arquivo");
        return -1;
    }

    return 0;
}

void processar_pacote(kermit_t *pacote) {
    // Desmonta o campo info
    uint8_t tamanho = pacote->info & 0b00111111; // Bits 0-5
    uint8_t sequencia = (pacote->info >> 6) & 0b00011111; // Bits 6-10
    uint8_t tipo = (pacote->info >> 11) & 0b00011111; // Bits 11-15

    // Exibe informações do pacote
    printf("Pacote recebido:\n");
    printf("Inicio: 0b%08b\n", pacote->inicio);
    printf("Info: Tamanho=%d, Sequência=%d, Tipo=0b%05b\n", tamanho, sequencia, tipo);
    printf("Dados: %.*s\n", tamanho, pacote->dados);
    printf("CRC: 0b%08b\n", pacote->crc);

    // Ações baseadas no tipo
    switch (tipo) {
        case 0b00100: // backup
            server_backup(pacote);
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

int main()
{
    const char *interface = "enp0s31f6"; // Interface para escutar
    int sockfd = create_raw_socket((char *)interface);

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
            processar_pacote(pacote);
        }
        else
        {
            printf("Pacote ignorado (campo de início inválido: 0b%08b)\n", pacote->inicio);
        }
    }

    return 0;
}
