#include "kermit.h"

void processa_pacote(kermit_t *pacote) {
    switch (get_tipo(pacote))
    {
    case ACK:
        puts("Received ACK from server");
        break;
    case NACK:
        puts("Received NACK from server");
        break;
    }
}

int client_backup(char *filename, int sockfd, struct sockaddr_ll addr)
{
    kermit_t sender;
    char buffer[1024];

    montar_pacote(BACKUP, &sender, filename, strlen(filename), 0);
    sendto(sockfd, &sender, sizeof(kermit_t), 0, (struct sockaddr *)&addr, sizeof(addr));

    recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr *)&addr, (socklen_t *)sizeof(addr));
    kermit_t *receiver = (kermit_t *)buffer;

    processa_pacote(receiver);

    if (get_tipo(receiver) == NACK)
        return -1;

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
    
    while (1)
    {
        char *command, *filename;

        printf("$ ");
        fgets(buffer, 1024, stdin);

        command = strtok(buffer, " ");
        filename = strtok(NULL, " ");

        command[strcspn(command, "\n")] = '\0';
        filename[strcspn(filename, "\n")] = '\0';

        if (strcmp(command, "backup") == 0)
        {
            client_backup(filename, sockfd, addr);
        }
    }

    return 0;
}
