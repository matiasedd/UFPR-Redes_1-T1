#include "protocol.h"

#define MAX_BUF_SIZE 1024

int main() {
    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (sockfd < 0) {
        perror("Erro ao criar socket");
        return 1;
    }

    struct sockaddr_ll server;
    memset(&server, 0, sizeof(server));
    
    server.sll_family = AF_PACKET;
    server.sll_protocol = htons(ETH_P_ALL);
    server.sll_ifindex = if_nametoindex("lo");

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Erro ao bindar o socket");
        return 1;
    }

    char buffer[MAX_BUF_SIZE];

    printf("Listening...\n");
    while (1) {
        struct sockaddr_ll client;
        socklen_t len = sizeof(client);
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client, &len);

        if (n > 0) {
            kermit_package_t *msg = (kermit_package_t *)buffer;
            // Calcula o CRC e verifica

            printf("RECEIVED\n%s\n", msg->data);
        }
    }

    close(sockfd);
    return 0;
}
