#include "kermit.h"

uint16_t seq = -1;

int client_backup(char *filename, int sockfd)
{
    kermit_t pacote;
    int timeout;
    FILE *arquivo = fopen(filename, "r");

    if (arquivo == NULL)
    {
        perror("Erro ao abrir arquivo");
        return -1;
    }

    // backup
    timeout = -1;
    do {
        montar_pacote(BACKUP, &pacote, filename, strlen(filename), ++seq);
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

    timeout = -1;
    do {
        montar_pacote(TAMANHO, &pacote, buffer, (uint16_t) tam, ++seq);
        enviar_pacote(&pacote, sockfd);
        timeout = receber_pacote(&pacote, sockfd);
    } while (timeout == -1);

    // dados
    size_t bytes;

    while ((bytes = fread(buffer, sizeof(uint8_t), 63, arquivo)))
    {
        timeout = -1;
        do {
            montar_pacote(DADOS, &pacote, buffer, bytes, ++seq);
            enviar_pacote(&pacote, sockfd);
            timeout = receber_pacote(&pacote, sockfd);
        } while (timeout == -1);
    }

    // finaliza
    timeout = -1;
    do {
        montar_pacote(FINALIZA, &pacote, NULL, 0, ++seq);
        enviar_pacote(&pacote, sockfd);
        timeout = receber_pacote(&pacote, sockfd);
    } while (timeout == -1);

    fclose(arquivo);

    puts("backup finalizado");

    return 0;
}

/* --- Restaura --- */


int client_restaura(char *filename, int sockfd) 
{
    return 0;
}

/* --- Verifica --- */

int validar_pacote(kermit_t *pacote) 
{
    return 1;
}

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
            goto restaura_timeout;          /*  Solicita denovo */

        case 0:                            /* Recebeu um pacote */
            //if(!validar_pacote(&pacote))
            //    goto restaura_timeout;

            imprime_pacote(&pacote);

            tipo = get_tipo(&pacote);

            switch (tipo) {
                case ERRO:
                    puts("[client_verfica]: Arquivo nao encontrado\n");
                    break;
                case NACK:
                    puts("[client_verfica]: Nack. Pedindo Novamente");
                    goto restaura_timeout;
                case OK_CHECKSUM:
                    printf("[client_verifica]: CheckSum: %s\n\n", pacote.dados);
                    break;
                default:
                    printf("Nao consigo identificar\n");
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

#if DEBUG
    puts("[main]: Iniciando IO com o usuario");
#endif

    while (1)
    {
        printf("$ ");
        fgets(buffer, 1024, stdin);

        command = strtok(buffer, " ");
        filename = strtok(NULL, "\n");

        putchar('\n');

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

