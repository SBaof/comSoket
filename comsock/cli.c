#include "sckutil.h"
#include "commsock.h"

int main(int argc, const char *argv[])
{
    int ret = 0;
    void *handle = NULL;
    unsigned char *data = (unsigned char*)"asdfghjklwertyuio";
    unsigned char recvbuf[1024];
    int recvbuflen = 1024;
    int datalen = 10;
    int connfd;

    ret = sckClient_init(&handle, 15, 5, 5, 1);
    if(ret < 0)
    {
        printf("func sckClient_init err\n");
        return ret;
    }

    ret = sckClient_getconn(handle, "127.0.0.1", 8888, &connfd);

    sckClient_send(handle, connfd, (unsigned char*)data, datalen);
    sckClient_recv(handle, connfd, (unsigned char*)recvbuf, &recvbuflen);
    sckClient_send(handle, connfd, (unsigned char*)data, datalen);

    sckClient_recv(handle, connfd, (unsigned char*)recvbuf, &recvbuflen);

    recvbuf[recvbuflen] = '\0';
    printf("recvbuf:%s\n", recvbuf);

    sckClient_destroy(handle);

    sckClient_closeconn(connfd);

    return 0;
}
