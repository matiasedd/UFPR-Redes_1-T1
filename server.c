#include "kermit.h"

uint16_t seq = -1;

void server_backup(kermit_t *receiver, int sockfd)
{
    kermit_t pacote;

    // montar_pacote(OK, &pacote, NULL, 0, ++seq);
    // enviar_pacote(&pacote, sockfd);

    // tamanho

    while (get_tipo(&pacote) != FINALIZA)
    {
        receber_pacote(&pacote, sockfd);
    }

#if DEBUG
    puts("backup finalizado");
#endif
}

void server_verifica(kermit_t *pacote, int sockfd)
{
    char buff[1024];

    sprintf(buff, "/usr/bin/cksum %s", pacote->dados);

#if DEBUG
    printf("[server_verifica]: Calculando o checksum de %s\n", pacote->dados);
#endif

    FILE *fp = popen(buff, "r");

    char buff_cksum[63]; /* evitar warnings do gcc */
    memset(buff, 0, 63);

    buff_cksum[0] = 'C';

    fgets(buff_cksum, 63, fp);

    printf("[server_verifica]: Retorno da cksum: %s", buff_cksum);

    pclose(fp);

    if (buff_cksum[0] == 'C')  /* Arquivo nao encontrado */
    {
        puts("[server_verifica]: Arquivo Nao Encontrado. Enviando Erro");
        montar_pacote(ERRO, pacote, NULL, 0, 1);
    }

    else
    {
        puts("[server_verifica]: Enviando Checksum");
    	montar_pacote(OK_CHECKSUM, pacote, buff_cksum, strlen(buff_cksum)-1, 1);
    }

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
            break;
        }
    }

    return 0;
}
