/*
   created at 2015//4/9
   latest modified at 2015/4/9
   test server used by alexjlz
 */

#include "tcpip/tcpip.h"
#include "utils/utils.h"
#include "core/core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    int fd, n = 0;
    struct packet *buff = (struct packet*)malloc(sizeof(struct packet));
    bzero(buff, sizeof(*buff));

    fd = connect_tcp_server("127.0.0.1", 21337);
    while ( (n = read(fd, buff, sizeof(struct packet))) > 0 )
    {
        //buff[sizeof(struct packet)] = 0;
        //fputs(buff, stdout);
        parse_packet(buff);
    }
    close(fd);

    return 0;
}
