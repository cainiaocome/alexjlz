/*
   created at 2015//4/9
   latest modified at 2015/4/9
   network functions used by alexjlz
 */

// create a listen socket, with bind to (inaddr_any, port)
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int create_tcp_server(short port);
int connect_tcp_server(char *ip, short port);
ssize_t readn(int fd, void *vptr, size_t n);
ssize_t writen(int fd, const void *vptr, size_t n);
int get_remote_ip(int fd, char remote_ip[]);
