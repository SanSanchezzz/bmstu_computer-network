#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "types.h"
#include "converter.h"

int sock;

void close_app()
{
    if (close(sock) < 0)
        exit(-1);

    printf("\nSocket closed\n");
    exit(0);
}

int main()
{
    size_t len;
    char *str;
    struct sockaddr_in addr;

    signal(SIGINT, close_app);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    while (1)
    {
        printf("Type number: ");
        getline(&str, &len, stdin);
        send(sock, str, strlen(str), 0);
    }

    close(sock);

    return 0;
}
