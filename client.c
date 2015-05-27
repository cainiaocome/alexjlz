/*
   created at 2015//4/9
   latest modified at 2015/4/9
   test server used by alexjlz
 */

#include "tcpip/tcpip.h"
#include "utils/utils.h"
#include "core/core.h"
#include "alg/alexjlz_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    int fd, n = 0;

    fd = connect_tcp_server("172.16.48.1", 21337);
    if ( ask_for_service( fd ) == -1 )
    {
        fprintf(stdout, "error in client\n");
    }

    if ( check_fd(fd) )
    {
        close(fd);
    }

    return 0;
}
