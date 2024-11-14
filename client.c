#include "protocol.h"

void send_package(int sockfd, struct sockaddr_ll *server, kermit_package_t *msg) {
    // Calcula o CRC

    if (sendto(sockfd, msg, sizeof(kermit_package_t), 0, (struct sockaddr *)server, sizeof(*server)) < 0) {
        perror("Erro ao enviar pacote");
    }
}

void backup(int sockfd, struct sockaddr_ll *servidor, const char *nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    
    if (!arquivo) {
        perror("Erro ao abrir arquivo");
        return;
    }

    // Variáveis de controle
    uint8_t sequencia = 0;
    kermit_package_t msg;
    size_t bytes_lidos;

    // Loop para enviar o arquivo em pedaços de 63 bytes
    while ((bytes_lidos = fread(msg.data, 1, MAX_DATA_SIZE, arquivo)) > 0) {
        msg.mark = 0x7E;  // Marcador de início fixo
        msg.size = bytes_lidos;   // Tamanho dos dados
        msg.seq = sequencia++; // Incrementa a sequência
        msg.type = 0x10;             // Tipo 'dados'

        send_package(sockfd, servidor, &msg);
    }

    fclose(arquivo);
    printf("Backup completo!\n");
}


int main() {
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (sockfd < 0) {
        perror("Erro ao criar socket");
        return 1;
    }

    struct sockaddr_ll server;
    memset(&server, 0, sizeof(server));

    server.sll_family = AF_PACKET;
    server.sll_protocol = htons(ETH_P_ALL);
    server.sll_ifindex = if_nametoindex("lo");

    char *command = "backup";
    char *filename = "Makefile";

    if (strcmp(command, "backup") == 0)
        backup(sockfd, &server, filename);

    close(sockfd);

    return 0;
}
