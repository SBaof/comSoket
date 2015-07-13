#include "sckutil.h"

void do_client(int conn)
{
    char recvbuf[1024] = {0};
    char sendbuf[1024] = {0};

    int ret = 0;

    fd_set rset;
    FD_ZERO(&rset);

    int fd_stdin = fileno(stdin);
    int maxfd = conn;
    int nready;
    maxfd = MAXFD(conn, fd_stdin);
    printf("maxfd:%d, conn:%d, fd_stdin:%d\n", maxfd, conn, fd_stdin);

    int stdineof = 0;
    while(1)
    {
        if(stdineof == 0)
        {
            FD_SET(fd_stdin, &rset);
        }
        FD_SET(conn, &rset);

        nready = select(maxfd+1, &rset, NULL, NULL, NULL);
        if(nready == -1)
        {
            ERR_EXIT("select");
        }
        if(nready == 0)
        {
            continue;
        }

        if(FD_ISSET(conn, &rset))
        {
            int ret = readline(conn, recvbuf, sizeof(recvbuf));
            if(ret == -1)
            {
                ERR_EXIT("readline");
            }
            else if(ret == 0)
            {
                printf("server closed\n");
                break;
            }
            fputs(recvbuf, stdout);
            memset(recvbuf, 0, sizeof(recvbuf));
        }
        if(FD_ISSET(fd_stdin, &rset))
        {
            if(fgets(sendbuf, sizeof(sendbuf), stdin) == NULL)
            {
                printf("ctr+d caused fgets return NULL\n");
                stdineof = 1;
                close(conn);
            }
            else
            {
                writen(conn, sendbuf, strlen(sendbuf));
                memset(sendbuf, 0, sizeof(sendbuf));
            }
        }
    }

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
