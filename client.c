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



int client_restaura(char *filename, int sockfd) 
{
    return 0;
}

int client_verifica(char *filename, int sockfd) 
{
    return 0;
}

int client_shell(char *command) 
{
    char buff[1024];

    sprintf(buff, "/usr/bin/%s ", command);


    FILE *shell = popen(buff, "r");
    
    if (shell == NULL)
    {
        perror("Comando Invalido");
        return -1;
    }

    while(fgets(buff, 1024, shell))
        printf("    %s", buff);

    pclose(shell);

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
        else if (strcmp(command, "verifica") == 0) 
        {
            client_verifica(filename, sockfd);
        }
        else if (strcmp(command, "restaura") == 0)
        {
            client_restaura(filename, sockfd);
        }
        else
        {
            client_shell(command);
        }
    }

    return 0;
}
