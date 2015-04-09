/*
   created at 2015//4/9
   latest modified at 2015/4/9
   test server used by alexjlz
 */

#include "tcpip/tcpip.h"
#include "utils/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

int main(int argc, char **argv)
{
    int listen_fd, connect_fd = 0;
    int status = 0;
    struct sockaddr_storage client;
    socklen_t client_len;
    char buff[1024];

    client_len = sizeof client;

    listen_fd = create_tcp_server(21337);
    printf("listen_fd: %d\n", listen_fd);
    printf("client_len: %d\n", client_len);

    status = check_fd(1);
    printf("status of fd 1: %d\n", status);
    status = check_fd(listen_fd);
    printf("status of fd listen_fd: %d\n", status);

    for ( ; ; )
    {
        //connect_fd = accept(listen_fd, (struct sockaddr *)&client, &client_len);
        connect_fd = accept(listen_fd, NULL, NULL);
        printf("got connected!\n");
        if ( connect_fd == -1)
        {
            perror("accept");
            exit(-1);
        }
        bzero(buff, sizeof buff);
        if( (char *)alexjlz_time(buff) == NULL)
        {
            perror("alexjlz_time");
            exit(-1);
        }
        printf("write to client: %s\n", buff);
        write(connect_fd, buff, strlen(buff));

        close(connect_fd);
    }

    return 0;
}
