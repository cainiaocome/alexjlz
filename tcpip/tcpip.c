/*
   created at 2015//4/9
   latest modified at 2015/4/9
   network functions used by alexjlz
 */

#include "tcpip.h"
#include <sys/socket.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int create_tcp_server(short port)
{
    int listen_fd = 0;
    struct sockaddr_in server_addr;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        perror("socket");
        exit(-1);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in)) == -1)
    {
        perror("bind");
        exit(-1);
    }

    if (listen(listen_fd, 3) == -1 )
    {
        perror("listen");
        exit(-1);
    }

    return listen_fd;
}

int connect_tcp_server(char *ip, short port)
{
    int fd = 0;
    struct sockaddr_in server_addr;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("socket");
        exit(-1);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if ( inet_pton(AF_INET, ip, &(server_addr.sin_addr) ) < 0 )
    {
        perror("inet_pton");
        exit(-1);
    }

    if ( connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 )
    {
        perror("connect");
        exit(-1);
    }

    return fd;
}

ssize_t						/* Read "n" bytes from a descriptor. */
readn(int fd, void *vptr, size_t n)  /* Return bytes read */
{
	size_t	nleft;
	ssize_t	nread;
	char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nread = read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR)
				nread = 0;		/* and call read() again */
			else
				return(-1);
		} else if (nread == 0)
			break;				/* EOF */

		nleft -= nread;
		ptr   += nread;
	}
	return(n - nleft);		/* return >= 0 */
}
/* end readn */

ssize_t						/* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}
/* end writen */

