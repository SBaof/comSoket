#ifndef _SCK_CLINT_H_
#define _SCK_CLINT_H_

#ifdef _cplusplus
extern 'C'
{
#endif

#define Sck_OK    0
#define Sck_BaseErr    3000

#define Sck_ErrParam      (Sck_BaseErr+1) //参数错误
#define Sck_ErrTimeOut    (Sck_BaseErr+2) //超时
#define Sck_ErrPeerClosed (Sck_BaseErr+3) //对方关闭连接
#define Sck_ErrMalloc     (Sck_BaseErr+4) //分配内存错误

//客户端初始化
//int sckClient_init(void **handle, char *ip, int port, int contime, int sendtime, int recvtime);
//int sckClient_init(void **handle, int contime, int sendtime, int recvtime, int countNum);
int sckClient_init(void **handle, int contime, int sendtime, int recvtime, int nCountNum);

//客户端连接
int sckClient_getconn(void *handle, char *ip, int port, int *connfd);

//客户端关闭连接
int sckClient_closeconn(int connfd);
int sckClient_putconn(int *connfd);

//客户端发送报文
int sckClient_send(void *handle, int connfd, unsigned char *data, int datalen);

//客户端接收报文
int sckClient_recv(void *handle, int connfd, unsigned char *data, int *datalen);

//客户端环境释放
int sckClient_destroy(void *handle);

#ifdef _cpluspluse
}
#endif

#endif
