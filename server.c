/*
   created at 2015//4/9
   latest modified at 2015/4/11
   test server used by alexjlz
 */

#include "tcpip/tcpip.h"
#include "utils/utils.h"
#include "core/core.h"
#include "log/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>  // signal
#include <unistd.h>  // daemon
#include <netinet/in.h>

int main(int argc, char **argv)
{
    pid_t pid = 0;
    int listen_fd, connect_fd = 0;
    int status = 0;
    struct sockaddr_storage client;
    socklen_t client_len;
    char time[1024];
    struct packet *buff = (struct packet*)malloc(sizeof(struct packet));

    client_len = sizeof client;

    // at now we dont need it 
    if (daemon(0, 0) < 0)
    {
        perror("daemon");
        exit(-1);
    }

    signal(SIGCHLD, SIG_IGN); // if we ignore SIGCHLD, we dont have to wait for child, and it will not become zombie.
                                // but this is not the best way.
    listen_fd = create_tcp_server(21337);

    for ( ; ; )
    {
        //connect_fd = accept(listen_fd, (struct sockaddr *)&client, &client_len);
        connect_fd = accept(listen_fd, NULL, NULL);

        alexjlz_log("got connected! connect_fd: %u\n", connect_fd);
        if ( connect_fd == -1)
        {
            perror("accept");
            exit(-1);
        }

        pid = fork();
        if ( pid < 0 )          // fork error
        {
            perror("fork");
            exit(-1);
        }
        else if ( pid == 0 )    // fork child
        {
            if( alexjlz_time(time) == NULL)
            {
                perror("alexjlz_time");
                exit(-1);
            }
            bzero(buff, sizeof(*buff));
            buff = make_packet(1, (unsigned long)strlen(time), time, buff);

            write(connect_fd, buff, sizeof(*buff));
            /*  this will generate error
            if ( buff != NULL)
            {
                free(buff);
            }
            */

            close(connect_fd);

            alexjlz_log("child %u exiting...\n", getpid());
            break;
            alexjlz_log("this should never appear!\n");
        }
        else if ( pid > 0 )  // todo: we need to wait for our child process, or it will become zombie
        {
            close(connect_fd);
            continue;            
        }
    }

    return 0;
}
