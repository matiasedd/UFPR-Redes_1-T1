#include "protocol.h"

#define MAX_BUF_SIZE 1024

int main() {
    /* --- Handle Socket --- */

    puts("Iniciando Socket");

    int sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (sockfd < 0) {
        perror("Erro ao criar socket");
        return 1;
    }

    struct sockaddr_ll client_addr;

    memset(&client_addr, 0, sizeof (client_addr));
    client_addr.sll_family = AF_PACKET;
    client_addr.sll_protocol = htons(ETH_P_ALL);
    client_addr.sll_ifindex = if_nametoindex(DEVICE);

    if (bind(sockfd, (struct sockaddr *) &client_addr, sizeof (client_addr)) < 0) {
        perror("Erro no bind");
        return -2;
    }

    puts("Socket Iniciado com Sucesso");

    /* --- Handle Pkg --- */

    kermit_package_t pkg;
    uint8_t seq_ctl = 0;

    printf("Escutando...\n");

    for (;;) {
        puts("recv");
        recv(sockfd, &pkg, sizeof (pkg), 0);

        ++seq_ctl;
    }

    close(sockfd);
    return 0;
}
