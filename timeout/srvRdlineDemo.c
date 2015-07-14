#include "sckutil.h"
#include <sys/wait.h>

void sig_child(int signum)
{
    pid_t pid;
    while((pid = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        printf("connect cut down,pid:%d\n", pid);
    }
}

void do_server(int conn)
{
    int ret = 0;
    char recvbuf[1024] = {0};

    while(1)
    {
        memset(recvbuf, 0, sizeof(recvbuf));
        ret = readline(conn, recvbuf, sizeof(recvbuf));
        if(ret == -1)
        {
            ERR_EXIT("readline");
        }
        if(ret == 0)
        {
            printf("client closed\n");
            break;
        }

        fputs(recvbuf, stdout);

        writen(conn, recvbuf, strlen(recvbuf));

        memset(recvbuf, 0, sizeof(recvbuf));
    }
    close(conn);
}

int main(int argc, const char *argv[])
{
    signal(SIGCHLD, sig_child);
    signal(SIGPIPE, SIG_IGN);
    if(argc < 3)
    {
        printf("Usage: %s ip_addr ip_port\n", argv[0]);
    }

    const char *ip = argv[1];
    unsigned int port = atoi(argv[2]);
    int listenfd;

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ERR_EXIT("socket");
    }

    SAIN srvaddr;
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &srvaddr.sin_addr);
    if(bind(listenfd, (SA*)&srvaddr, sizeof(srvaddr)) < 0)
    {
        ERR_EXIT("bind");
    }

    if(listen(listenfd, 5) < 0)
    {
        ERR_EXIT("listen");
    }

    pid_t pid;
    int conn;
    SAIN clnaddr;
    int clnaddrlen = sizeof(clnaddr);

    while(1)
    {
        if((conn = accept(listenfd, (SA*)&clnaddr, &clnaddrlen)) < 0)
        {
            ERR_EXIT("accept");
        }

        printf("connected with:ip:%s, port:%d\n", inet_ntoa(clnaddr.sin_addr),
                htons(clnaddr.sin_port));

        pid = fork();
        if(pid == -1)
        {
            ERR_EXIT("fork");
        }
        else if(pid == 0)
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
    return 0;
}
