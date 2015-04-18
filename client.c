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
    unsigned long hash = 0;
    char value_buff[1024] = {0};
    struct packet buff;
    bzero((char *)&buff, sizeof(buff));

    fd = connect_tcp_server("127.0.0.1", 21337);

    make_register_packet(&buff);
    fprintf(stdout, "register uuid:%s\n", buff.value);

    writen(fd, &buff, sizeof(buff));

    readn(fd, &buff, sizeof(buff));
    make_response_packet(&buff);
    writen(fd, &buff, sizeof(buff));

    close(fd);

    return 0;
}
