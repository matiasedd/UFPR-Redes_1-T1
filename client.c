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
    nome_arq[strlen(nome_arq)-1] = '\0';
    
    /* BACKUP */
    kermit_t pacote;

    FILE *arquivo = fopen(nome_arq, "r");

    if (arquivo == NULL) {
        perror("Erro ao abrir arquivo");
        return -1;
    }

    montar_pacote(&pacote, nome_arq, 1, 0b00100);

    // Envio do pacote
    ssize_t bytes_enviados = sendto(sockfd, &pacote, sizeof(kermit_t), 0,
                                    (struct sockaddr*)&destino, sizeof(destino));
    if (bytes_enviados == -1) {
        perror("Erro ao enviar pacote");
        return -1;
    }

    printf("Pacote enviado com sucesso (%ld bytes)\n", bytes_enviados);

    fclose(arquivo);

    return 0;
}
