#include "kermit.h"


int client_backup(char *filename, int sockfd)
{
    FILE *arquivo = fopen(filename, "r");

    if (arquivo == NULL)
    {
        perror("[cliente_backup]: Erro ao abrir arquivo");
        return -1;
    }

    kermit_t pacote;
    int timeout;
    int seq = 0;

    // backup, receber ack
    do {
        montar_pacote(BACKUP, &pacote, filename, strlen(filename), seq++);
        enviar_pacote(&pacote, sockfd);
        timeout = receber_pacote(&pacote, sockfd);
    } while (timeout == -1);

    // tamanho
    fseek(arquivo, 0, SEEK_END);
    long int tam = ftell(arquivo);

    fseek(arquivo, 0, SEEK_SET);

    char buffer[255];
    sprintf(buffer, "%ld", tam);

    if (tam)
        tam = floor(log10((double)tam) + 1);

    do {
        montar_pacote(TAMANHO, &pacote, buffer, (uint16_t) tam, seq++);
        enviar_pacote(&pacote, sockfd);
        timeout = receber_pacote(&pacote, sockfd);
    } while (timeout == -1);

    // dados
    size_t bytes;

    while ((bytes = fread(buffer, sizeof(uint8_t), 63, arquivo)))
    {
        do {
            montar_pacote(DADOS, &pacote, buffer, bytes, seq++);
            enviar_pacote(&pacote, sockfd);
            timeout = receber_pacote(&pacote, sockfd);
        } while (timeout == -1);
    }

    // finaliza
    puts("mandando finaliza");

    do {
        montar_pacote(FINALIZA, &pacote, NULL, 0, seq++);
        enviar_pacote(&pacote, sockfd);
        timeout = receber_pacote(&pacote, sockfd); /* Recebe o ultimo ack */
    } while (timeout == -1);

    fclose(arquivo);

    puts("[client_backup]: backup finalizado\n");

    return 0;
}

/* --- Restaura --- */


int client_restaura(char *filename, int sockfd) 
{
    FILE *arquivo = fopen(filename, "w");

    kermit_t pacote;
    int timeout;
    int seq = 0;

    uint16_t tipo;

    /* envia requisicao */

    do {
        montar_pacote(RESTAURA, &pacote, filename, strlen(filename), seq);
        enviar_pacote(&pacote, sockfd);
        timeout = receber_pacote(&pacote, sockfd);
    } while (timeout == -1);

    ++seq;

    /* recebe tamanho, envia ack */

    do {
        montar_pacote(ACK, &pacote, NULL, 0, seq);
        enviar_pacote(&pacote, sockfd);
        timeout = receber_pacote(&pacote, sockfd);
    } while (timeout == -1);

    ++seq;

    /* recebe dados, envia ack */

    while (1)
    {
        do {
          timeout = receber_pacote(&pacote, sockfd);
        } while (timeout == -1);

        tipo = get_tipo(&pacote);

        if (tipo == FINALIZA)
        {
            puts("Finalizando transmissao");
        }

        else if (tipo == DADOS) 
        {
            fprintf(arquivo, "%s", pacote.dados);
        }

        montar_pacote(ACK, &pacote, NULL, 0, seq++);
        enviar_pacote(&pacote, sockfd);
    }

    /* envia finaliza */

    montar_pacote(ACK, &pacote, NULL, 0, seq);
    enviar_pacote(&pacote, sockfd);

    ++seq;

    fclose(arquivo);

    puts("[client_restaura]: Restaura Finalizado");

    return 0;
}

/* --- Verifica --- */

int client_verifica(char *filename, int sockfd) 
{
    kermit_t pacote;
    uint16_t sequencia;
    uint16_t tipo;

    restaura_timeout:

    sequencia = 0;

    montar_pacote(VERIFICA, &pacote, filename, strlen(filename), sequencia++);
    enviar_pacote(&pacote, sockfd);

    switch(receber_pacote(&pacote, sockfd)) {
        case -1:                            /* Timeout */
            puts("[client_verifica]: Timeout - Sending again");
            sequencia = 0;
            goto restaura_timeout;          /*  Solicita novamente */

        case 0:                            /* Recebeu um pacote */
            imprime_pacote(&pacote);

            tipo = get_tipo(&pacote);

            switch (tipo) 
            {
                case ERRO:
                    puts("[client_verfica]: Arquivo nao encontrado\n");
                    break;
                case NACK:
                    puts("[client_verfica]: Nack. Pedindo Novamente");
                    goto restaura_timeout;
                case OK_CHECKSUM:
                    printf("[client_verifica]: CheckSum: %s\n\n", pacote.dados);
                    break;
            }
    }

    return 0;
}


/* --- Outros --- */

int client_shell(char *command) 
{
    char buff[1024];

    sprintf(buff, "/usr/bin/%s ", command);

    FILE *shell = popen(buff, "r");
    
    if (shell == NULL)
    {
        perror("[client_shell]: Comando Invalido");
        return -1;
    }

    while (fgets(buff, 1024, shell))
          printf("    %s", buff);

    pclose(shell);

    return 0;
}

/* --- Main --- */

int main()
{
    int sockfd = create_raw_socket(NET_INTERFACE);

    char buffer[1024];
    char *command, *filename;

    /* --- Handle Input --- */

    puts("[main]: Iniciando IO com o usuario");

    while (1)
    {
        printf("$ ");
        fgets(buffer, 1024, stdin);

        command = strtok(buffer, " ");
        filename = strtok(NULL, "\n");

        putchar('\n');

        if (strcmp(command, "exit") == 0) 
        {
          return 0;
        }
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

    return 1;
}

