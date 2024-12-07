#include "kermit.h"

int server_backup(kermit_t *pacote, int sockfd, char *buffer, struct sockaddr_ll addr)
{
    // char *filename = (char *)pacote->dados;
    // FILE *arquivo = fopen("filename", "w");

    // if (arquivo == NULL)
    //     return -1;

    recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *)&addr, (socklen_t *)sizeof(addr));
    kermit_t *receiver = (kermit_t *)buffer;

    imprime_pacote(receiver);

    // fclose(arquivo);

    return 0;
}

int main()
{
    int sockfd = create_raw_socket(NET_INTERFACE);

    struct sockaddr_ll addr = {0};
    addr.sll_family = AF_PACKET;
    addr.sll_ifindex = if_nametoindex(NET_INTERFACE);
    addr.sll_protocol = htons(ETH_P_ALL);

    char buffer[1024];
    kermit_t sender;

    puts("The server has been initalized. Waiting for the packets...");

    while (1)
    {
        recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *)&addr, (socklen_t *)sizeof(addr));
        kermit_t *receiver = (kermit_t *)buffer;

        imprime_pacote(receiver);
        
        switch (get_tipo(receiver))
        {
        case BACKUP:
            puts("Sending OK to client");
            montar_pacote(OK, &sender, NULL, 0, 0);
            sendto(sockfd, &sender, sizeof(kermit_t), 0, (struct sockaddr *)&addr, sizeof(addr));

            int response = server_backup(receiver, sockfd, buffer, addr);

            if (response == 0)
            {
                puts("Sending OK to client");
                montar_pacote(OK, &sender, NULL, 0, 0);
                sendto(sockfd, &sender, sizeof(kermit_t), 0, (struct sockaddr *)&addr, sizeof(addr));
            }
            else
            {
                puts("Sending NACK to client");
                montar_pacote(NACK, &sender, NULL, 0, 0);
                sendto(sockfd, &sender, sizeof(kermit_t), 0, (struct sockaddr *)&addr, sizeof(addr));
            }

            break;
        }
    }

    return 0;
}
