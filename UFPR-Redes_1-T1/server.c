#include "kermit.h"


void server_backup(kermit_t *receiver, int sockfd)
{
    kermit_t pacote;

    char *filename = (char*) receiver->dados;
    FILE *arquivo = fopen(filename, "w");

    int timeout;
    int seq = 0;

    montar_pacote(OK, &pacote, NULL, 0, seq++);
    enviar_pacote(&pacote, sockfd);

    // tamanho
    do {
    	timeout = receber_pacote(&pacote, sockfd);
    } while (timeout == -1);

    montar_pacote(OK, &pacote, NULL, 0, seq++);
    enviar_pacote(&pacote, sockfd);

    // dados
    uint16_t tipo;

    while (1)
    {
        do {
            timeout = receber_pacote(&pacote, sockfd);
        } while (timeout == -1);

        tipo = get_tipo(&pacote);

        printf("%hhu\n", tipo);

        if (tipo == FINALIZA)	
        {
            puts("Finalizando transmissao");
            break;
	      }

        else if (tipo == DADOS)
        {
            fprintf(arquivo, "%s", pacote.dados);
        }

        montar_pacote(ACK, &pacote, NULL, 0, seq++);
        enviar_pacote(&pacote, sockfd);
    }

    montar_pacote(ACK, &pacote, NULL, 0, seq++); /* envaidno o ultimo ack */
    enviar_pacote(&pacote, sockfd);

    fclose(arquivo);

    puts("[server_backup]: Backup Finalizado");
}

void server_verifica(kermit_t *pacote, int sockfd)
{
    char buff[1024];

    sprintf(buff, "/usr/bin/cksum %s", pacote->dados);

    printf("[server_verifica]: Calculando o cksum de %s\n", pacote->dados);

    FILE *fp = popen(buff, "r");

    char buff_cksum[63];
    memset(buff_cksum, 0, 63);

    buff_cksum[0] = 'C';

    fgets(buff_cksum, 63, fp);

    printf("[server_verfica]: Retorno da cksum: %s", buff_cksum);

    pclose(fp);

    if (buff_cksum[0] == 'C')
    {
        puts("[server_verifica]: Arquivo Nao encontrado. Enviando Erro");
        montar_pacote(ERRO, pacote, NULL, 0, 1);
    }

    else 
    {
      puts("[server_verifica]: Enviando cksum");
      montar_pacote(OK_CHECKSUM, pacote, buff_cksum, strlen(buff_cksum) - 1, 1);
    }

    enviar_pacote(pacote, sockfd);
}

void server_restaura(kermit_t *pkg, int sockfd)
{
    char filename[1024];

    snprintf(filename, get_tamanho(pkg), "%s", pkg->dados);

    printf("[server_restaura]: Restaurando %s\n", filename);

    FILE *arquivo = fopen(filename, "r");

    if (arquivo)
        puts("foi aberto");

  /*
  if (arquivo == NULL)
  {
      perror("[server_restaura]: Erro ao abrir arquivo");
      return; 
  }
  */

    kermit_t pacote;
    int timeout;
    int seq = 0;

    fseek(arquivo, 0, SEEK_END);
    long int tam = ftell(arquivo);
    fseek(arquivo, 0, SEEK_SET);

    char buffer[255];
    sprintf(buffer, "%ld", tam);

    if (tam)
        tam = floor(log10((double)tam) + 1);

    puts("tamanho");

    do {
        montar_pacote(TAMANHO, &pacote, buffer, (uint16_t) tam, seq++);
        enviar_pacote(&pacote, sockfd);
        timeout = receber_pacote(&pacote, sockfd);
    } while (timeout == -1);

    puts("dados");

    size_t bytes;

    while ((bytes = fread(buffer, sizeof(uint8_t), 63, arquivo)))
    {
        do {
            montar_pacote(DADOS, &pacote, buffer, bytes, seq++);
            enviar_pacote(&pacote, sockfd);
            timeout = receber_pacote(&pacote, sockfd);
        } while (timeout == -1);
    }

    puts("mandando finaliza");

    do {
        montar_pacote(FINALIZA, &pacote, NULL, 0, seq++);
        enviar_pacote(&pacote, sockfd);
        timeout = receber_pacote(&pacote, sockfd);
    } while (timeout == -1);

    fclose(arquivo);

    puts("[server_restaura]: Restaura Finalizado\n");

    return;
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
            server_restaura(&pacote, sockfd);
            puts("[main]: Fim server_restaura\n");
            break;
        default:
            break;
        }
    }

    return 0;
}

