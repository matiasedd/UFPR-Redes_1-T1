#include "kermit.h"

void processa_pacote(kermit_t *pacote)
{
    switch (get_tipo(pacote))
    {
    case OK:
        puts("Received OK from server");
        break;
    case ACK:
        puts("Received ACK from server");
        break;
    case NACK:
        puts("Received NACK from server");
        break;
    }
}

int client_backup(char *filename, int sockfd)
{
    FILE *arquivo = fopen(filename, "r");

    if (arquivo == NULL) {
        perror("Erro ao abrir arquivo");
        return -1;
    }

    kermit_t pacote;
    uint16_t seq = 0;

    montar_pacote(BACKUP, &pacote, filename, strlen(filename), seq++);
    enviar_pacote(&pacote, sockfd);

    return 0;
}

int main()
{
    int sockfd = create_raw_socket(NET_INTERFACE);

    char buffer[1024];
    char *command, *filename;

    /* --- Handle Input --- */

#if DEBUG
    puts("Iniciando IO com o usuario");
#endif

    while (1)
    {
        printf("$ ");
        fgets(buffer, 1024, stdin);

        command = strtok(buffer, " ");
        filename = strtok(NULL, "\n");

        if (strcmp(command, "backup") == 0)
        {
            client_backup(filename, sockfd);
        }
    }

    return 0;
}
