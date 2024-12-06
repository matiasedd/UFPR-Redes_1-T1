#include "kermit.h"

void montar_pacote(kermit_t *pacote, const char *mensagem, uint8_t sequencia, uint8_t tipo)
{
    memset(pacote, 0, sizeof(kermit_t));

    pacote->inicio = INICIO;
    uint8_t tamanho = strlen(mensagem);
    if (tamanho > 64)
        tamanho = 64;

    pacote->info = (tamanho & OFFSET_6) | ((sequencia & OFFSET_5) << 6) | ((tipo & OFFSET_5) << 11);

    memcpy(pacote->dados, mensagem, tamanho);

    pacote->crc = 0x0;
}

void processar_pacote(kermit_t *pacote)
{
    // Desmonta o campo info
    uint8_t tamanho = pacote->info & OFFSET_6;          // Bits 0-5
    uint8_t sequencia = (pacote->info >> 6) & OFFSET_5; // Bits 6-10
    uint8_t tipo = (pacote->info >> 11) & OFFSET_5;     // Bits 11-15

    switch(tipo) {
        case OK:
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
    const char *interface = "lo";
    int sockfd = create_raw_socket((char *)interface);

    struct sockaddr_ll destino = {0};
    destino.sll_family = AF_PACKET;
    destino.sll_ifindex = if_nametoindex(interface);
    destino.sll_protocol = htons(ETH_P_ALL);

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

    montar_pacote(&pacote, nome_arq, 1, BACKUP);

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
    if (pkg->inicio == INICIO)
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
