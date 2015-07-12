#include "sckutil.h"

void do_server(int conn)
{
    packet recvbuf;
    int n;
    while(1)
    {
        memset(&recvbuf, 0, sizeof(recvbuf));

        int ret = readn(conn, &recvbuf.len, 4);
        if(ret == -1)
        {
            ERR_EXIT("readn");
        }
        else if(ret < 4)
        {
            printf("client closted\n");
            break;
        }

        n = ntohl(recvbuf.len);
        ret = readn(conn, recvbuf.buf, n);
        if(ret == -1)
        {
            ERR_EXIT("readn");
        }
        else if(ret < n)
        {
            printf("client closed\n");
            break;
        }

        fputs(recvbuf.buf, stdout);

        writen(conn, &recvbuf, 4+n);
    }
}

int main(int argc, const char *argv[])
{
    if(argc < 3)
    {
        printf("Usage: ./%s ip_addr ip_port\n", argv[0]);
        return -1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int listenfd;
    int ret = 0;

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ERR_EXIT("socket");
    }

    struct sockaddr_in srvaddr;
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htonl(port);
    inet_pton(AF_INET, ip, &srvaddr.sin_addr);

    if(bind(listenfd, (struct sockaddr*)&srvaddr, sizeof(srvaddr)) < 0)
    {
        ERR_EXIT("bind");
    }

    if(listen(listenfd, 5) < 0)
    {
        ERR_EXIT("listen");
    }

    int conn;
    int pid;
    struct sockaddr_in *clnaddr;
    int clnaddr_len = sizeof(clnaddr);

    while(1)
    {
        /*
        conn = accept_timeout(listenfd, clnaddr, 5);
        if(conn == -1 && errno == ETIMEDOUT)
        {
            ERR_EXIT("accept");
        }
        else if(conn == -1)
        {
            ERR_EXIT("accept");
        }*/
        conn = accept(listenfd, (struct sockaddr*)&clnaddr, &clnaddr_len);
        if(connect < 0)
        {
            ERR_EXIT("connect");
        }

        if((pid = fork()) == -1)
        {
            ERR_EXIT("fork");
        }
        if(pid == 0)
        {
            close(listenfd);
            do_server(conn);
            exit(0);
        }
        else
        {
            close(conn);
        }
    }

    return 0;
}

