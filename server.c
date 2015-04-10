/*
   created at 2015//4/9
   latest modified at 2015/4/9
   test server used by alexjlz
 */

#include "tcpip/tcpip.h"
#include "utils/utils.h"
#include "core/core.h"
#include "log/log.h"

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
    char time[1024];
    struct packet *buff = (struct packet*)malloc(sizeof(struct packet));

    client_len = sizeof client;

    listen_fd = create_tcp_server(21337);

    for ( ; ; )
    {
        //connect_fd = accept(listen_fd, (struct sockaddr *)&client, &client_len);
        connect_fd = accept(listen_fd, NULL, NULL);

        alexjlz_log("got connected!\n");
        if ( connect_fd == -1)
        {
            perror("accept");
            exit(-1);
        }

        if( alexjlz_time(time) == NULL)
        {
            perror("alexjlz_time");
            exit(-1);
        }
        bzero(buff, sizeof(*buff));
        buff = make_packet(1, (unsigned long)strlen(time), time, buff);

        write(connect_fd, buff, sizeof(*buff));
        /*
        if ( buff != NULL)
        {
            free(buff);
        }
        */

        close(connect_fd);
    }

    return 0;
}
