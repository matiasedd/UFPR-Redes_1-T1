#include "protocol.h"

int main() {
    /* --- Handle Socket --- */

    puts("Iniciando Socket");

    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (sockfd < 0) {
        perror("Erro ao criar socket");
        return -1;
    }

    struct sockaddr_ll server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sll_family = AF_PACKET;
    server_addr.sll_protocol = htons(ETH_P_ALL);
    server_addr.sll_ifindex = if_nametoindex(DEVICE);

    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof (server_addr)) < 0) {
      perror("Erro no bind");
      return -2;
    }

    puts("Socket Iniciado com Sucesso");

    /* --- Handle Input --- */

    puts("Iniciando IO com o usuario");

    char input_buffer[INPUT_BUFFER_SIZE];
    const char *DELIM = " ";
    char *command, *filename;

    for (;;) {
        printf("Input: ");

        fgets(&input_buffer[0], INPUT_BUFFER_SIZE, stdin);

        command = strtok(&input_buffer[0], DELIM);
        filename = strtok(NULL, "\n");

        switch(*command) {
          case 'b': client_backup(sockfd, &server_addr, filename); break;
          case 'r': break;
          case 'v': break;
          default:  
            puts("Entrada Invalida");
            break; 
        }
    }

    close(sockfd);

    return 0;
}

/* --- backup, restaura e verifica --- */

void client_backup(int sockfd, struct sockaddr_ll *server_addr, const char *pathname) {
    FILE *file;

    puts("Iniciando Backup");

    if (!(file = fopen(pathname, "r"))) {
        perror("Erro ao abrir arquivo - (Path Invalido, Permisao Negada ou Arquivo Inexistente)");
        return;
    }

    kermit_package_t pkg;
    uint8_t seq_ctl = 0;
    uint16_t size;

    /* --- Nome --- */

    /* --- Tamanho --- */

    /* --- Backup --- */

    puts("Iniciando Transmisao");

    pkg.mark = 0x7E;  // Marcador de início fixo

    while ((size = fread(pkg.data, 1, MAX_DATA_SIZE, file)) > 0) {

        sendto(sockfd, &pkg, sizeof (kermit_package_t), 0, (struct sockaddr *)server_addr, sizeof (*server_addr));
        ++seq_ctl;
    }

    /* --- Fim Tx --- */

    fclose(file);

    printf("Backup completo!\n");
}

void restaura() {
  return;
}

void verifica() {
  return;
}

