#include "sckutil.h"

void do_client(int sockfd)
{
    packet sendbuf;
    packet recvbuf;
    memset(&sendbuf, 0, sizeof(sendbuf));
    memset(&recvbuf, 0, sizeof(recvbuf));

    int n;
    while(fgets(sendbuf.buf, sizeof(sendbuf.buf), stdin)!=NULL)
    {
        n = strlen(sendbuf.buf);
        //printf("strlen(sendbuf.buf): %d\n", strlen(sendbuf.buf));
        //printf("sizeof(sendbuf.buf): %d\n", sizeof(sendbuf.buf));
        sendbuf.len = htonl(n);
        writen(sockfd, &sendbuf, 4+n);

        int ret = readn(sockfd, &recvbuf.len, 4);
//        printf("ret:%d\n", ret);
        if(ret == -1)
        {
            ERR_EXIT("readn");
        }
        else if(ret<4)
        {
            printf("client closed\n");
            //break;
            exit(EXIT_FAILURE);
        }

        readn(sockfd, recvbuf.buf, n);

        printf("get: ");
        fputs(recvbuf.buf, stdout);
        memset(&sendbuf, 0, sizeof(sendbuf));
        memset(&recvbuf, 0, sizeof(recvbuf));
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
//    int srvaddr_len = sizeof(srvaddr);

    ret = connect(sockfd, (struct sockaddr *)&srvaddr, sizeof(srvaddr));
    //printf("ret:%d\n", ret);
    if(ret < 0)
    {
        ERR_EXIT("connect");
    }

    do_client(sockfd);
    close(sockfd);

    return 0;
}
