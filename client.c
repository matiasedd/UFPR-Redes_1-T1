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
    kermit_t sender;
    char buffer[1024];

    FILE *arquivo = fopen(filename, "r");

    if (arquivo == NULL)
    {
        perror("Erro ao abrir arquivo");
        return -1;
    }

    puts("Sending BACKUP to server");
    montar_pacote(BACKUP, &sender, filename, strlen(filename), 0);
    send(sockfd, &sender, sizeof(kermit_t), 0);

    recv(sockfd, buffer, 1024, 0);
    kermit_t *receiver = (kermit_t *)buffer;

    processa_pacote(receiver);

    if (get_tipo(receiver) == NACK)
        return -1;

    // tamanho
    puts("Sending TAMANHO to server");
    /* v1
    fseek(arquivo, 0, SEEK_END);
    uint8_t size = ftell(arquivo);

    montar_pacote(TAMANHO, &sender, size, sizeof(uint8_t), 0);
    send(sockfd, &sender, sizeof(kermit_t), 0);
    */
    //struct stat info;
    //stat(filename, &info);

    // montar_pacote(TAMANHO, &sender, (uint8_t *)info.st_size, sizeof(uint8_t), 0);
    // imprime_pacote(&sender);

    // dados

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
