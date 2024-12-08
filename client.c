#include "kermit.h"

uint16_t seq = -1;

int client_backup(char *filename, int sockfd)
{
    FILE *arquivo = fopen(filename, "r");

    if (arquivo == NULL)
    {
        perror("Erro ao abrir arquivo");
        return -1;
    }

    kermit_t pacote;

    // backup
    montar_pacote(BACKUP, &pacote, filename, strlen(filename), ++seq);
    enviar_pacote(&pacote, sockfd);

    // receber_pacote(&pacote, sockfd);

    // tamanho
    fseek(arquivo, 0, SEEK_END);
    long int tam = ftell(arquivo);
    fseek(arquivo, 0, SEEK_SET);

    char buffer[255];
    sprintf(buffer, "%ld", tam);

    if (tam)
        tam = floor(log10((double)tam) + 1);

    montar_pacote(TAMANHO, &pacote, buffer, (uint16_t) tam, ++seq);
    enviar_pacote(&pacote, sockfd);

    // dados
    size_t bytes;

    while ((bytes = fread(buffer, sizeof(uint8_t), 63, arquivo)))
    {
        montar_pacote(DADOS, &pacote, buffer, bytes, ++seq);
        enviar_pacote(&pacote, sockfd);
    }

    // finaliza

    montar_pacote(FINALIZA, &pacote, NULL, 0, ++seq);
    enviar_pacote(&pacote, sockfd);

    fclose(arquivo);

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
