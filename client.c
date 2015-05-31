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

int main(int argc, char **argv)
{
    int fd, n = 0;

    generate_uuid();

    daemonize();
    while ( 1 )
    {
        fd = connect_tcp_server("self.1isp.cc", 21337);
        if ( ask_for_service( fd ) == -1 )
        {
            fprintf(stdout, "error in client\n");
        }
        
        if ( check_fd(fd) )
        {
            close(fd);
        }

        sleep(3);
    }

    return 0;
}
