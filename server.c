#include "kermit.h"

void server_backup(kermit_t *receiver, int sockfd)
{
#if DEBUG
    puts("backup iniciado");
#endif
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

    kermit_t pacote;

    while (1)
    {
        receber_pacote(&pacote, sockfd);

        switch (get_tipo(&pacote))
        {
        case BACKUP:
            server_backup(&pacote, sockfd);
            break;
        case VERIFICA:
            server_verifica();
            break;
        case RESTAURA:
            server_restaura();
            break;
        // default:
        //     puts("invalid");
        //     break;
        }
    }

    return 0;
}
