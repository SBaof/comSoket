#ifndef _SOCK_UTIL_H_
#define _SOCK_UTIL_H_

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define ERR_EXIT(m) \
    do \
    { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } \
    while(0)

typedef struct packet_
{
    int len;
    char buf[1024];
}packet;

void activate_nonnblock(int fd);
void deactivate_nonblock(int fd);

int read_timeout(int fd, unsigned int wait_seconds);
int write_timeout(int fd, unsigned int wait_seconds);
int accept_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds);
int connect_timeout(int fd, struct sockaddr_in *addr, unsigned int wait_seconds);

ssize_t readn(int fd, void *buf, size_t count);
ssize_t writen(int fd, void *buf, size_t count);
ssize_t recv_peek(int sockfd, void *buf, size_t len);
ssize_t readline(int sockfd, void *buf, size_t maxlen);

#endif /*_SYS_UTIL_H_*/
