#include "kermit.h"

int server_backup_wrapper(kermit_t *pacote, int sockfd, char *buffer)
{
    //char *filename = (char *)pacote->dados;
    //FILE *arquivo = fopen(
    //if (arquivo == NULL)
    //    return -1;
    
    recv(sockfd, buffer, 1024, 0);
    kermit_t *receiver = (kermit_t *) buffer;

    imprime_pacote(receiver);

    //fclose(arquivo)
    
    return 0;
}

void server_backup(kermit_t *receiver, int sockfd)
{	
    char buffer[1024];
    kermit_t sender;

    puts("Sending OK to client");
    montar_pacote(OK, &sender, NULL, 0, 0);
    send(sockfd, &sender, sizeof(kermit_t), 0);

    int response = server_backup_wrapper(receiver, sockfd, buffer);

    if (response == 0)
    {
        puts("Sending OK to client");
        montar_pacote(OK, &sender, NULL, 0, 0);
        send(sockfd, &sender, sizeof(kermit_t), 0);
    }
    else
    {
        puts("Sending NACK to client");
        montar_pacote(NACK, &sender, NULL, 0, 0);
        send(sockfd, &sender, sizeof(kermit_t), 0);
    }

    return;
}

void server_verifica()
{

}

void server_restaura()
{

}

int main()
{
    int sockfd = create_raw_socket(NET_INTERFACE);

    char buffer[1024];

    while (1)
    {
        recv(sockfd, buffer, 1024, 0);
        kermit_t *receiver = (kermit_t *)buffer;

        switch (get_tipo(receiver))
        {
        case BACKUP:
            server_backup(receiver, sockfd);
            break;
        case VERIFICA:
            server_verifica();
            break;
        case RESTAURA:
            server_restaura();
            break;
        default:
            puts("invalid");
            break;
      }
    }

    return 0;
}
