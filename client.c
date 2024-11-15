#include "protocol.h"

int main() {
    /* --- Handle Socket --- */

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

    /* --- Handle Input --- */

    #define INPUT_BUFFER_SIZE 1024
    char input_buffer[INPUT_BUFFER_SIZE];
    const char *DELIM = " ";
    char *command, *filename;

    while (1) {
        fgets(&input_buffer[0], INPUT_BUFFER_SIZE, stdin);

        command = strtok(&input_buffer[0], DELIM);
        filename = strtok(NULL, "\n");

        if (strcmp(command, "backup") == 0)
            backup(sockfd, &server, filename);
        else if (strcmp(command, "restaura") == 0)
            continue;
        else if (strcmp(command, "verifica") == 0)
            continue;
        else if (strcmp(command, "sair") == 0)
            break;
        else
          printf("Comando Invalido!\n");
    }

    close(sockfd);

    return 0;
}

/* --- backup, restaura e verifica --- */

void backup(int sockfd, struct sockaddr_ll *servidor, const char *nome_arquivo) {
    FILE *arquivo = fopen(nome_arquivo, "r");
    
    if (!arquivo) {
        perror("Erro ao abrir arquivo");
        return;
    }

    // Variáveis de controle

    kermit_package_t msg;
    uint8_t sequencia = 0;
    uint16_t bytes_lidos;

    /* --- Nome --- */

    /* --- Tamanho --- */

    /* --- Backup --- */

    while ((bytes_lidos = fread(msg.data, 1, MAX_DATA_SIZE, arquivo)) > 0) {
        msg.mark = 0x7E;  // Marcador de início fixo

        sendto(sockfd, &msg, sizeof(kermit_package_t), 0, (struct sockaddr *)servidor, sizeof(*servidor));
    }

    /* --- Fim Tx --- */


    fclose(arquivo);
    printf("Backup completo!\n");
}

