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
/*
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
        //writen(conn, recvbuf, sizeof(recvbuf));

        memset(recvbuf, 0, sizeof(recvbuf));
    }
    close(conn);
}*/

void do_server(int conn)
{
    long arg1, arg2;
    ssize_t n;
    char line[1024] = {0};

    while(1)
    {
        if((n = readline(conn, line, sizeof(line))) == 0)
        {
            return;
        }
        if(sscanf(line, "%ld%ld", &arg1, &arg2) == 2)
        {
            snprintf(line, sizeof(line), "%ld\n", arg1 + arg2);
        }
        else
        {
            snprintf(line, sizeof(line), "input error\n");
        }

        n = strlen(line);
        writen(conn, line, n);
        memset(line, 0, sizeof(line));
    }
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
