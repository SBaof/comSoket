#include "sckutil.h"
#include "commsock.h"

typedef struct SckHandle_
{
    int sockfdArray[100];
    int ArrayNum;
    int sockfd;
    int connfd;
    int conntime;
    int sendtime;
    int recvtime;
}SckHandle;

//客户端初始化
int sckClient_init(void **handle, int conntime,
        int sendtime, int recvtime, int nCountNum)
{
    int ret = 0;
    if(handle == NULL || conntime == 0 || sendtime == 0 || recvtime == 0)
    {
        ret = Sck_ErrParam;
        printf("func sckClient_init() err:check (handle ==NULL...) %d\n", ret);
        return ret;
    }

    SckHandle *tmp = (SckHandle *)malloc(sizeof(SckHandle));
    if(tmp == NULL)
    {
        ret = Sck_ErrMalloc;
        printf("func sckClient_init err: check malloc %d\n", ret);
        return ret;
    }

    /*
    int i = 0;
    for(i=0; i<nCountNum; i++)
    {
        int sock;
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if(sock < 0)
        {
            ret = errno;
            printf("func socket() err:%d\n", ret);
            return ret;
        }
        tmp->sockfdArray[i] = sock;
    }*/
    *handle = tmp;
    tmp->conntime = conntime;
    tmp->sendtime = sendtime;
    tmp->recvtime = recvtime;
    tmp->ArrayNum = nCountNum;
    return 0;
}

int sckClient_getconn(void *handle, char *ip, int port, int *connfd)
{
//    SckHandle *tmp = handle;
    int ret = 0;
    if(handle == NULL || ip == NULL || port < 0 || port > 65536 || connfd == NULL)
    {
        ret = Sck_ErrParam;
        printf("func sckClient_getconn() err:check (handle == NULL...%d\n", ret);
        return ret;
    }

    SAIN servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &servaddr.sin_addr);

    int sock ;//= tmp->sockfd;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        ret = errno;
        printf("func socket() err:%d\n", ret);
        return ret;
    }

    ret = connect(sock, (SA*)&servaddr, sizeof(servaddr));
    if(ret < 0)
    {
        ret = errno;
        printf("func connect() err: %d\n", errno);
        return ret;
    }
    *connfd = sock;
    return ret;
}

//客户端发送报文
int sckClient_send(void *handle, int connfd, unsigned char *data, int datalen)
{
    //if(handle == NULL || connfd ==0 || data == NULL ||  )
    int ret = 0;
    SckHandle *tmp = NULL;
    tmp = (SckHandle *)handle;

    ret = write_timeout(connfd, tmp->sendtime);
    if(ret == 0)
    {
        unsigned char *netdata = (unsigned char *)malloc(datalen + 4);
        if(netdata == NULL)
        {
            ret = Sck_ErrMalloc;
            printf("func sckClient_send() malloc err:%d", ret);
            return ret;
        }

        int len = htonl(datalen);
        memcpy(netdata, &len, 4);
        memcpy(netdata + 4, data, datalen);

        int writed = 0;
        writed = writen(connfd, netdata, datalen + 4);
        if(writed < datalen +4)
        {
            if(netdata != NULL)
            {
                free(netdata);
                netdata = NULL;
            }
            return writed;
        }
    }
    if(ret == -1)
    {
        if(errno == ETIMEDOUT)
        {
            ret = Sck_ErrTimeOut;
            printf("func sckClient_send() err:%d\n", ret);
            return ret;
        }
        return ret;
    }

    return 0;
}

//客户端接收报文
int sckClient_recv(void *handle, int connfd, unsigned char *data, int *datalen)
{
    int ret = 0;
    SckHandle *temp = (SckHandle *)handle;

    if(handle==NULL || data==NULL)
    {
        ret = Sck_ErrParam;
        printf("func SckClient_rcv() err:%d\n", ret);
        return ret;
    }

    ret = read_timeout(connfd, temp->sendtime);
    if(ret != 0)
    {
        if(ret <0 && errno == ETIMEDOUT)
        {
            ret = Sck_ErrTimeOut;
            printf("func SckClient_rcv() timeout,err:%d\n", ret);
            return ret;
        }
        else
        {
            printf("func SckClient_rcv() err:%d\n", ret);
            return ret;
        }
    }

    int netdatalen = 0;
    ret = readn(connfd, &netdatalen, 4);
    if(ret == -1)
    {
        printf("func readn() err:%d\n", ret);
        return ret;
    }
    else if(ret <4)
    {
        ret = Sck_ErrPeerClosed;
        printf("func readn() err peer closed:%d\n", ret);
        return ret;
    }

    int n = 0;
    n = ntohl(netdatalen);
    ret = readn(connfd, data, n);
    if(ret == -1)
    {
        printf("func readn() err:%d\n", ret);
        return ret;
    }
    else if(ret < n)
    {
        ret = Sck_ErrPeerClosed;
        printf("func readn() err peer closed:%d\n", ret);
        return ret;
    }

    *datalen = n;

    return 0;
}

//客户端环境释放
int sckClient_destroy(void *handle)
{
    if(handle != NULL)
    {
        free(handle);
    }
    return 0;
}

int sckClient_closeconn(int connfd)
{
    if(connfd >= 0)
    {
        close(connfd);
    }
    return 0;
}
