#include "sckutil.h"
#include <sys/wait.h>

void myhandle(int sig)
{
    int pid = 0;
    if(sig == SIGCHLD)
    {
        while((pid = waitpid(-1, NULL, WNOHANG)) > 0)
        {
            printf("connect cut down, pid:%d\n", pid);
        }
    }
}

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
        else if(ret<4)
        {
            printf("client closed\n");
            break;
        }

        n = ntohl(recvbuf.len);
        ret = readn(conn, recvbuf.buf, n);
        if(ret == -1)
        {
            ERR_EXIT("readn");
        }
        else if(ret < n )
        {
            printf("client closed\n");
            break;
        }

//        printf("1.recvbuf.len=%d\n", n);

        fputs(recvbuf.buf, stdout);

        writen(conn, &recvbuf, 4+n);

        //printf("recvbuf.len=%d\n", recvbuf.len);
    }
}

int main(int argc, const char *argv[])
{
    signal(SIGCHLD, myhandle);

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

    int on = 1;
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        ERR_EXIT("setsockopt");
    }

    struct sockaddr_in srvaddr;
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(port);
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
    struct sockaddr_in clnaddr;
    int clnaddr_len = sizeof(clnaddr);

    while(1)
    {
        /*conn = accept_timeout(listenfd, clnaddr, 5);
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
            ERR_EXIT("accept");
        }

        printf("connected with ip:%s, port:%d\n", inet_ntoa(clnaddr.sin_addr),
                                                ntohs(clnaddr.sin_port));

        if((pid = fork()) == -1)
        {
            ERR_EXIT("fork");
        }
        if(pid == 0)
        {
            close(listenfd);
            do_server(conn);
            exit(EXIT_SUCCESS);
        }
        else
        {
            close(conn);
        }
    }

    close(conn);
    close(listenfd);
    return 0;
}

