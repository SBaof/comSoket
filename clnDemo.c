#include "sckutil.h"

void do_client(int sockfd)
{
    int ret;
    int n;
    packet recvbuf, sendbuf;

    memset(&recvbuf, 0, sizeof(recvbuf));
    memset(&sendbuf, 0, sizeof(sendbuf));

    while(fgets(sendbuf.buf, sizeof(sendbuf.buf), stdin) != NULL)
    {
        n = strlen(sendbuf.buf);
        sendbuf.len = htonl(n);
        writen(sockfd, &sendbuf, 4+n);

        ret = readn(sockfd, &recvbuf.len, 4);
        if(ret < 0)
        {
            ERR_EXIT("readn");
        }
        else if(ret<4)
        {
            printf("server closed\n");
            break;
        }
        n = ntohl(recvbuf.len);
        readn(sockfd, recvbuf.buf, n);

        fputs(recvbuf.buf, stdout);

        memset(&recvbuf, 0, sizeof(recvbuf));
        memset(&sendbuf, 0, sizeof(sendbuf));
    }
    close(sockfd);
}

int main(int argc, const char *argv[])
{
    if(argc < 2)
    {
        printf("Usage:./%s ip_address ip_port\n", argv[0]);
        return -1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd;
    int ret;

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ERR_EXIT("socket");
    }

    struct sockaddr_in srvaddr;
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &srvaddr.sin_addr);
    int srvaddr_len = sizeof(srvaddr);

    ret = connect(sockfd, (struct sockaddr *)&srvaddr, sizeof(srvaddr));
    //printf("ret:%d\n", ret);
    if(ret < 0)
    {
        ERR_EXIT("connect");
    }

    do_client(sockfd);

    return 0;
}
