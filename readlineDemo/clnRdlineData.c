#include "sckutil.h"

/*
void do_client(int conn)
{
    char recvbuf[1024] = {0};
    char sendbuf[1024] = {0};

    int ret = 0;

    while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
    {
        writen(conn, sendbuf, strlen(sendbuf));
        //writen(conn, sendbuf, sizeof(sendbuf));

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
}*/

void do_client(int conn)
{
    char sendbuf[1024];
    args args;
    result result;
    while(fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
    {
        if(sscanf(sendbuf, "%ld%ld", &args.arg1, &args.arg2) != 2)
        {
            printf("invalid input: %s\n", sendbuf);
            continue;
        }

        //writen(conn, sendbuf, sizeof(sendbuf));
        writen(conn, &args, sizeof(args));
        printf("args.arg1:%ld, args.arg2:%ld\n", args.arg1, args.arg2);
        if(readn(conn, &result, sizeof(result)) == 0)
        {
            printf("str_cli:server terminated prematurely");
            ERR_EXIT("readn");
        }
        printf("result: %ld\n", result.sum);
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
