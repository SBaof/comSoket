#include "sckutil.h"

int main(int argc, const char *argv[])
{
    const char *ip = argv[1];
    int sock;
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        ERR_EXIT("socket");
    }

    //SAIN addr;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    socklen_t len = sizeof(addr);

    int ret = 0;
    ret = connect_timeout(sock, (struct sockaddr*)&addr, len, 5);
    //ret = connect(sock, (SA *)&addr, sizeof(addr));
    if(ret == -1 && errno == ETIMEDOUT)
    {
        printf("commect timeout...\n");
        return -1;
    }
    else if(ret == -1)
    {
        ERR_EXIT("connect_timeout");
    }

    SAIN localaddr;
    socklen_t addrlen = sizeof(localaddr);
    if(getsockname(sock, (SA*)&localaddr, &addrlen) < 0)
        ERR_EXIT("getsockname");

    printf("ip:%s,port:%d\n", inet_ntoa(localaddr.sin_addr), ntohs(localaddr.sin_port));

    close(sock);
    return 0;
}
