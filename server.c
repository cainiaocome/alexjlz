/*
   created at 2015/4/9
   latest modified at 2015/5/19
   alexjlz serve_clientr used by alexjlz
   ( I have never thought it would go that far :-) )
 */

#include "tcpip/tcpip.h"
#include "utils/utils.h"
#include "core/core.h"
#include "log/log.h"
#include "daemon/daemon.h"
#include "adt/list.h"
//#include "client.h"
//#include "alexjlz.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>  // signal
#include <unistd.h>  // daemon
#include <pthread.h> // pthread
#include <netinet/in.h>
#include <sys/select.h>
#include <errno.h>

#undef max
#define max(x,y) ((x) > (y) ? (x) : (y))

extern list_p client_list;

int main(int argc, char **argv)
{
    int client_listen_fd, client_connect_fd = 0;  // for client to connect
    int alexjlz_listen_fd, alexjlz_connect_fd = 0; // for alexjlz to connect and control , todo
    pthread_t tid = 0;
    int status = 0;

    client_list = create_list();

    while ( 1 )
    {
        if ( daemonize() == 0 )
            break;
        sleep(3);
    }
    alexjlz_log("daemonize success\n");
    //signal(SIGCHLD, SIG_IGN); // if we ignore SIGCHLD, we dont have to wait for child, and it will not become zombie.
    // but this is not the best way.
    Signal(SIGCHLD, sig_child);  // ( in utils ) this is better
    signal(SIGPIPE, SIG_IGN);

    client_listen_fd = create_tcp_server(21337);
    alexjlz_listen_fd = create_tcp_server(21338);
    alexjlz_log("client listener created.\n");
    alexjlz_log("alexjlz listener created.\n");

    fd_set fs;
    struct timeval tv;
    int retval;

    for ( ; ; )
    {
        FD_ZERO(&fs);
        FD_SET(client_listen_fd, &fs);
        FD_SET(alexjlz_listen_fd, &fs);
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        errno = 0;
        status = 0;
        retval = 0;
        retval = select( max(client_listen_fd, alexjlz_listen_fd)+1, &fs, NULL, NULL, &tv);
        if ( retval == -1 )  // error
        {
            if ( errno == EINTR ) // handle interrupted system call
                continue;
            else                 // anything else indicate error
            {
                alexjlz_log("Error, select:%s\n", strerror(errno));
                exit(-1);
            }
        }
        else if ( retval == 0)  // not ready
        {
            continue;
        }
        else   // ready
        {
            if ( FD_ISSET(client_listen_fd, &fs) )
            {
                if ( (client_connect_fd = accept(client_listen_fd, NULL, NULL)) <= 0 )    // big hole...
                {
                    alexjlz_log("Error, accept after select:%s\n", strerror(errno));
                }
                else
                {
                    while ( (status=pthread_create(&tid, NULL, &serve_client, &client_connect_fd)) == EAGAIN ) {}
                    if ( status < 0 )          // pthread_create error
                    {
                        alexjlz_log("pthread_create error:%s\n", strerror(errno));
                    }
                    else
                    {
                        pthread_detach(tid);
                        continue;            
                    }
                }
            }
            if ( FD_ISSET(alexjlz_listen_fd, &fs) )
            {
                if ( (alexjlz_connect_fd = accept(alexjlz_listen_fd, NULL, NULL)) <= 0 )    // big hole...
                {
                    alexjlz_log("Error, accept after select:%s\n", strerror(errno));
                }
                else
                {
                    while ( (status=pthread_create(&tid, NULL, &serve_alexjlz, &alexjlz_connect_fd)) == EAGAIN ) {}
                    if ( status < 0 )          // pthread_create error
                    {
                        alexjlz_log("pthread_create error:%s\n", strerror(errno));
                    }
                    else
                    {
                        pthread_detach(tid);
                        continue;            
                    }
                }
            }
        }
    }

    return 0;
}
