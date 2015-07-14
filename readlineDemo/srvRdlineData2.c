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
    ssize_t n;
    args args;
    result result;
    args.arg1 = 0;
    args.arg2 = 0;
    result.sum = 0;
    while(1)
    {
        if( (n = readn(conn, &args, sizeof(args))) == 0)
        {
            return ;
        }

        //printf("args.arg1:%ld, args.arg2:%ld\n", args.arg1,args.arg2);
        result.sum  = args.arg1 + args.arg2;
        //printf("result.sum=%ld\n", result.sum);
        writen(conn, &result, sizeof(result));
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
