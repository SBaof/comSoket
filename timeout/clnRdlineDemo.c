#include "sckutil.h"

void do_client(int conn)
{
    char recvbuf[1024] = {0};
    char sendbuf[1024] = {0};

    int ret = 0;

    while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
    {
        writen(conn, sendbuf, strlen(sendbuf));

        ret = readline(conn, recvbuf, sizeof(recvbuf));
        if(ret == -1)
        {
            ERR_EXIT("readline");
        }
        if(ret == 0)
        {
            printf("server closed\n");
            break;
        }

        fputs(recvbuf, stdout);

        memset(recvbuf, 0, sizeof(recvbuf));
        memset(sendbuf, 0, sizeof(sendbuf));
    }
    close(conn);
}

int main(int argc, const char *argv[])
{
    if(argc < 3)
    {
        printf("Usage:%s ip_addr ip_port\n", argv[0]);
    }

    const char *ip = argv[1];
    unsigned int port = atoi(argv[2]);
    int sockfd;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ERR_EXIT("socket");
    }

    SAIN srvaddr;
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &srvaddr.sin_addr);
    if(connect(sockfd, (SA*)&srvaddr, sizeof(srvaddr)) < 0)
    {
        ERR_EXIT("connect");
    }

    do_client(sockfd);
    close(sockfd);
    return 0;
}
