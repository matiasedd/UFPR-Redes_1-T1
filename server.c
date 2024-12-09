#include "kermit.h"

uint16_t seq = -1;

void server_backup(kermit_t *receiver, int sockfd)
{
    kermit_t pacote;

    montar_pacote(OK, &pacote, NULL, 0, ++seq);
    enviar_pacote(&pacote, sockfd);

    // tamanho
    receber_pacote(&pacote, sockfd);

    montar_pacote(OK, &pacote, NULL, 0, ++seq);
    enviar_pacote(&pacote, sockfd);

    while (get_tipo(&pacote) != FINALIZA)
    {
        receber_pacote(&pacote, sockfd);
        montar_pacote(ACK, &pacote, NULL, 0, ++seq);
        enviar_pacote(&pacote, sockfd);
    }

    /*
    do {
        receber_pacote(&pacote, sockfd);

        montar_pacote(ACK, &pacote, NULL, 0, ++seq);
        enviar_pacote(&pacote, sockfd);
    } while (get_tipo(&pacote) != FINALIZA);
    */

    puts("backup finalizado");
}

void server_verifica(kermit_t *pacote, int sockfd)
{
    char buff[1024];

    sprintf(buff, "/usr/bin/cksum %s", pacote->dados);

#if DEBUG
    printf("[server_verifica]: Calculando o checksum de %s\n", pacote->dados);
#endif

    FILE *fp = popen(buff, "r");

    if (fp == NULL)
    {
        puts("[server_verifica]: fp null");
        montar_pacote(ERRO, pacote, NULL, 0, 1);
        imprime_pacote(pacote);
        enviar_pacote(pacote, sockfd);

        return;
    }

    fgets(buff, 63, fp);
    snprintf((char *) pacote->dados, 63, "%s", buff);

    pclose(fp);

    montar_pacote(OK_CHECKSUM, pacote, buff, strlen(buff), 1);
    imprime_pacote(pacote);
    enviar_pacote(pacote, sockfd);
}

void server_restaura()
{
}

int main()
{
    int sockfd = create_raw_socket(NET_INTERFACE);

    kermit_t pacote;

    while (1)
    {
        receber_pacote(&pacote, sockfd);

        switch (get_tipo(&pacote))
        {
        case BACKUP:
            puts("\n[main]: Iniciando server_backup");
            server_backup(&pacote, sockfd);
            puts("[main]: Fim server_backup\n");
            break;
        case VERIFICA:
            puts("\n[main]: Iniciando server_verifica");
            server_verifica(&pacote, sockfd);
            puts("[main]: Fim server_verifica\n");
            break;
        case RESTAURA:
            puts("\n[main]: Iniciando server_restaura");
            server_restaura();
            puts("[main]: Fim server_restaura\n");
            break;
        default:
            //puts("[main]: Recebi um pacote Invalido");
            break;
        }
    }

    return 0;
}
