/*
   created at 2015//4/9
   latest modified at 2015/4/23
   alexjlz server used by alexjlz
   ( I have never thought it would go that far :-) )
 */

#include "tcpip/tcpip.h"
#include "utils/utils.h"
#include "core/core.h"
#include "log/log.h"
#include "daemon/daemon.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>  // signal
#include <unistd.h>  // daemon
#include <netinet/in.h>
#include <errno.h>

int main(int argc, char **argv)
{
    pid_t pid = 0;
    int listen_fd, connect_fd = 0;
    int status = 0;
    struct sockaddr_storage client;
    socklen_t client_len;
    char time[1024];
    struct packet *buff = (struct packet*)malloc(sizeof(struct packet));

    //client_len = sizeof(client);

    // at now we dont need it 
    /*
    if (daemon(0, 0) < 0)
    {
        alexjlz_log("daemon error");
        exit(-1);
    }
    */
    daemonize();

    //signal(SIGCHLD, SIG_IGN); // if we ignore SIGCHLD, we dont have to wait for child, and it will not become zombie.
                                // but this is not the best way.
    Signal(SIGCHLD, sig_child);  // ( in utils ) this is better

    listen_fd = create_tcp_server(21337);
    alexjlz_log("listen_fd set to %d\n", listen_fd);

    for ( ; ; )
    {
        errno = 0;
        if ( (connect_fd = accept(listen_fd, NULL, NULL)) <= 0 )  
        {
            if ( errno == EINTR ) // handle interrupted system call
                continue;
            else                 // anything else indicate error
            {
                alexjlz_log("accept:%s\n", strerror(errno));
                exit(-1);
            }
        }

        alexjlz_log("got connected! connect_fd: %u\n", connect_fd);

        pid = fork();
        if ( pid < 0 )          // fork error
        {
            alexjlz_log("fork error");
            exit(-1);
        }
        else if ( pid == 0 )    // fork child
        {
            if ( check_fd(listen_fd) )
            {
                close(listen_fd);
            }

            serve (connect_fd);

            if( check_fd(connect_fd) )
            {
                close(connect_fd);
            }

            alexjlz_log("child %u exiting...\n", getpid());
            break;
            alexjlz_log("this should never appear!\n");
        }
        else          // todo: we need to wait for our child process, or it will become zombie
        {
            close(connect_fd);
            continue;            
        }
    }

    return 0;
}
