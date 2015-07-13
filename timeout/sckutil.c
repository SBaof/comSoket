#include "sckutil.h"

/**
 * read_timeout----读超时检测函数，不含读操作
 * @fd:文件描述符
 * @wait_timeout:等待超时秒数，如果为0,表示不检测超时
 * 成功（未超时）返回0，失败返回-1,超时返回-1并且errno == EINTR
 * */
int read_timeout(int fd, unsigned int wait_seconds)
{
    int ret = 0;
    if(wait_seconds > 0)
    {
        fd_set read_fdset;
        struct timeval timeout;

        FD_ZERO(&read_fdset);
        FD_SET(fd, &read_fdset);

        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;

        /**
         * select返回值三态
         * 1.若timeout时间到(超时)，没有检测到读事件 ret返回0
         * 2.若ret返回<0 && errno == EINTR 说明select被信号中断
         * 2-1 若返回-1则出错
         * 3.若ret返回值大于0表示read事件返回，返回事件发生的个数
         * */
        do{
            ret = select(fd+1, &read_fdset, NULL, NULL, &timeout);
        }while(ret < 0 && errno == EINTR);

        if(ret == 0)
        {
            errno = ETIMEDOUT;
            ret = -1;
        }
        else if(ret == 1)
        {
            ret = 0;
        }
    }
    return ret;
}

/**
 * write_timeout----写超时检测函数
 * @fd:文件描述符
 * @wait_second:等待超时秒数， 如果为0 表示不检测超时
 * 成功（未超时）返回0, 失败返回-1,超时返回-1并且errno == ETIMEDOUT
 * */
int write_timeout(int fd, unsigned int wait_seconds)
{
    int ret = 0;
    if(wait_seconds > 0)
    {
        fd_set write_fdset;

        FD_ZERO(&write_fdset);
        FD_SET(fd, &write_fdset);

        struct timeval wait_timeout;
        wait_timeout.tv_sec = wait_seconds;
        wait_timeout.tv_usec = 0;

        do{
            ret = select(fd+1, NULL, &write_fdset, NULL, &wait_timeout);
        }while(ret < 0 && errno == EINTR);

        if(ret == 0)
        {
            ret =-1;
            errno = ETIMEDOUT;
        }
        else if(ret == 1)
        {
            ret = 0;
        }
    }
    return ret;
}

/*accpt_timeout---accept超时检测函数
 * @fd:套接字文件描述符
 * @addr:输出参数，返回对方地址
 * @wait_seconds:等待超时秒数，如果0表示正常模式
 * 成功（未超时）返回一连接套接字，超时返回-1并且errno==ETIMEDOUT
 */

int accept_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
    int ret = 0;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    if(wait_seconds > 0)
    {
        fd_set accept_fdset;
        struct timeval timeout;
        FD_ZERO(&accept_fdset);
        FD_SET(fd, &accept_fdset);
        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;
        do{
            ret = select(fd+1, &accept_fdset, NULL, NULL, &timeout);
        }while(ret < 0 && errno == EINTR);
        if(ret == -1)
        {
            return -1;
        }
        else if(ret == 0)
        {
            errno = ETIMEDOUT;
            ret = -1;
        }
    }

    //一旦检测出有select事件发生，表示三次握手完成，客户端有新建立的连接
    //此时再次调用accept将不会阻塞
    if(addr != NULL)
        ret = accept(fd, (struct sockaddr *)&addr, &addrlen);//返回已连接套接字
    else
        ret = accept(fd, NULL, NULL);
    if(ret == -1)
        ERR_EXIT("accept");

    return ret;
}

/*
 * active_noblock----设置I/O为非阻塞状态模式
 * @fd:表示文件描述符
 */
void active_nonblock(int fd)
{
    int ret;
    int flags = fcntl(fd, F_GETFL);
    if(flags == -1)
        ERR_EXIT("fcntl");

    flags |= O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, flags);
    if(ret == -1)
        ERR_EXIT("fcntl");
}

/*
 * deactive_nonblock---设置I/O为阻塞模式
 * @fd:文件描述符
 */
void deactive_nonblock(int fd)
{
    int ret = 0;
    int flags = fcntl(fd, F_GETFL);
    if(flags == -1)
        ERR_EXIT("fcntl");

    flags &= ~O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, flags);
    if(ret == -1)
        ERR_EXIT("fcntl");
}

/**
 * connect_timeout----connect超时检测函数
 * @fd:文件描述符
 * @addr:要连接的对方地址
 * @wait_seconds:等待对方超时秒数，如果返回0表示正常模式
 * 成功（未超时）返回0, 失败返回-1,超时返回-1并且errno==EIMEDOUT
 */
/**
 * 原因：tcpip在客户端连接服务器的时候，如果异常，connect（默认fd是阻塞属性）
 * 返回时间是1.5*RTT，大约在75秒以上，会造成软件性能的下降
 *
 * 思想：先将fd的默认的阻塞属性设置为非阻塞模式，然后先试着connect，如果网络通畅
 * 将立即建立连接，如果网络不好，则根据返回值进行判断处理
 * if(ret<0 && errno == EPROGRESS)表示客户端与服务器正建立连接，先等一等
 * 等待的时间，我们可以自己控制，把select管理中心用上
 * 相当于通过select管理中心去监控sockfd的状态
 * 这样会大大提高我们的产品质量
 *
 * 注意：select机制监控到conn可度，并不能代表连接可用
 * 还需要进一步的判断（造成可读有几种情况：1.真正建立起连接；2.建立失败）
 * 通过int sockoptret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &socklen)
 * 做一个容错即可
 */
int connect_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds)
{
    int ret;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    if(wait_seconds > 0)
        active_nonblock(fd);

    ret = connect(fd, (struct sockaddr *)&addr, addrlen);
    if(ret < 0 && errno == EINPROGRESS)
    {
        fd_set connect_fdset;
        struct timeval timeout;
        FD_ZERO(&connect_fdset);
        FD_SET(fd, &connect_fdset);

        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;

        do{
            ret = select(fd+1, NULL, &connect_fdset, NULL, &timeout);
        }while(ret < 0 && errno == EINTR);
        if(ret == 0)
        {
            ret = -1;
            errno = ETIMEDOUT;
        }
        else if(ret < 0)
        {
            return -1;
        }
        else if(ret == 1)
        {
            int err;
            socklen_t socklen = sizeof(err);
            int sockopret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &socklen);
            if(sockopret == -1)
            {
                return -1;
            }
            if(err == 0)
            {
                ret = 0;
            }
            else
            {
                errno = err;
                ret = -1;
            }
        }
    }
    if(wait_seconds > 0)
    {
        deactive_nonblock(fd);
    }
    return ret;
}


ssize_t readn(int fd, void *buf, size_t count)
{
    ssize_t nread = 0;
    size_t nleft = count;
    void *bufp = buf;

    while(nleft > 0)
    {
        if((nread = read(fd, bufp, nleft)) < 0)
        {
            if(errno == EINTR)
                continue;
            return -1;
        }
	else if(nread == 0)
            return (nleft - nread);

        nleft -= nread;
        bufp += nread;
    }
    return count;
}

ssize_t writen(int fd, void *buf, size_t count)
{
    size_t nwrite;
    size_t nleft = count;
    void *bufp = buf;

    while(nleft > 0)
    {
        if((nwrite = write(fd, bufp, nleft)) < 0)
        {
            if(errno == EINTR)
                continue;
            return -1;
        }
        else if(nwrite == 0)
            continue;

        nleft -= nwrite;
        bufp += nwrite;
    }
    return count;
}

ssize_t recv_peek(int sockfd, void *buf, size_t len)
{
    int ret = 0;
    while(1)
    {
        ret = recv(sockfd, buf, len, MSG_PEEK);
        if(ret == -1 && errno == EINTR)
        {
            continue;
        }
        return ret;
    }
}

ssize_t readline(int sockfd,void *buf, size_t maxlen)
{
    int ret = 0;
    int nread;
    int nleft = maxlen;
    char *bufp = buf;

    while(1)
    {
        ret = recv_peek(sockfd, bufp, nleft);
        if(ret < 0)
        {
            return ret;
        }
        else if(ret == 0)
        {
            return ret;
        }

        nread = ret;
        int i = 0;
        for(i=0; i<nread; i++)
        {
            if(bufp[i] == '\n')
            {
                ret = readn(sockfd, bufp, i+1);
                if(ret != i+1)
                {
                    exit(EXIT_FAILURE);
                }
		return ret;
            }
        }

        if(nread > nleft)
        {
            exit(EXIT_FAILURE);
        }

        nleft -= nread;
        ret = readn(sockfd, bufp, nread);
        if(ret != nread)
        {
            exit(EXIT_FAILURE);
        }
        bufp += nread;
    }
    return -1;
}
