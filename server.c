#include "kermit.h"

int server_backup(kermit_t *pacote)
{
    char *filename = (char *)pacote->dados;
    FILE *arquivo = fopen(filename, "w");

    if (arquivo == NULL)
        return -1;

    fclose(arquivo);

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

    while (1)
    {
        recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *)&addr, (socklen_t *)sizeof(addr));
        kermit_t *receiver = (kermit_t *)buffer;

        switch (get_tipo(receiver))
        {
        case BACKUP:
            montar_pacote(ACK, &sender, NULL, 0, 0);
            sendto(sockfd, &sender, sizeof(kermit_t), 0, (struct sockaddr *)&addr, sizeof(addr));

            int response = server_backup(receiver);

            if (response == 0)
            {
                montar_pacote(ACK, &sender, NULL, 0, 0);
                sendto(sockfd, &sender, sizeof(kermit_t), 0, (struct sockaddr *)&addr, sizeof(addr));
            }
            else
            {
                montar_pacote(NACK, &sender, NULL, 0, 0);
                sendto(sockfd, &sender, sizeof(kermit_t), 0, (struct sockaddr *)&addr, sizeof(addr));
            }

            break;
        }
    }

    return 0;
}
