/*
   created at 2015//4/9
   latest modified at 2015/4/9
   network functions used by alexjlz
 */

#include "tcpip.h"
#include <sys/socket.h>

#include <sys/types.h>
#include <netinet/in.h>
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
