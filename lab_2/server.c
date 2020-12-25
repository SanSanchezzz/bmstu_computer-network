#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "converter.h"
#include "types.h"

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
    int sock;
    struct sockaddr_in addr;
    char buf[BUF_SIZE];
    int bytes_read;
    int res;

    signal(SIGINT, close_app);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(2);
    }

    while(1)
    {
        bytes_read = recvfrom(sock, buf, 1024, 0, NULL, NULL);
        if (bytes_read < 0)
        {
            printf("recvfrom error\n");
            return -1;
        }
        buf[bytes_read - 1] = '\0';

        res = converter(buf);
        if (res == ERROR)
        {
            return ERROR;
        }
    }
    
    return 0;
}
