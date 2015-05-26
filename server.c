/*
   created at 2015/4/9
   latest modified at 2015/5/19
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
#include <pthread.h> // pthread
#include <netinet/in.h>
#include <errno.h>

//extern int listen_fd;
//extern int connect_fd; // for client to connect and exchange msg
//extern int alexjlz_fd; // for alexjlz to connect and control

int listen_fd, connect_fd = 0;  // for client to connect
int alexjlz_fd = 0; // for alexjlz to connect and control


int main(int argc, char **argv)
{
    pthread_t tid = 0;
    int status = 0;
    struct sockaddr_storage client;
    socklen_t client_len;
    char time[1024];
    struct packet *buff = (struct packet*)malloc(sizeof(struct packet));

    //client_len = sizeof(client);

    daemonize();

    //signal(SIGCHLD, SIG_IGN); // if we ignore SIGCHLD, we dont have to wait for child, and it will not become zombie.
                                // but this is not the best way.
    Signal(SIGCHLD, sig_child);  // ( in utils ) this is better

    listen_fd = create_tcp_server(21337);
    alexjlz_fd = create_tcp_server(31337);
    alexjlz_log("listen_fd set to %d\n", listen_fd);
    alexjlz_log("alexjlz_fd set to %d\n", alexjlz_fd);

    for ( ; ; )
    {
        errno = 0;
        if ( (connect_fd = accept(listen_fd, NULL, NULL)) <= 0 )    // big hole...
        {
            if ( errno == EINTR ) // handle interrupted system call
                continue;
            else                 // anything else indicate error
            {
                alexjlz_log("accept:%s\n", strerror(errno));
                exit(-1);
            }
        }

        alexjlz_log("got client connecting! connect_fd: %u\n", connect_fd);

        //pid = fork();
        while ( (status=pthread_create(&tid, NULL, &serve, &connect_fd)) == EAGAIN ) {}
        if ( status < 0 )          // pthread_create error
        {
            alexjlz_log("pthread_create error:%s\n", strerror(errno));
            //pthread_cancel // to to
        }
        else
        {
            pthread_detach(tid);
            continue;            
        }
    }

    return 0;
}
