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
    char *uuid = "i love you so much";
    char value_buff[1024] = {0};
    struct packet *buff = (struct packet*)malloc(sizeof(struct packet));
    bzero(buff, sizeof(*buff));

    fd = connect_tcp_server("127.0.0.1", 21337);
    make_pre_register_packet(uuid, buff);
    fprintf(stdout, "register uuid:%s\n", buff->value);
    write(fd, buff, sizeof(struct packet));
    read(fd, buff, sizeof(struct packet));
    hash = alexjlz_hash(buff->value);

    snprintf(value_buff, 1024, "%lu", hash);
    make_pre_response_packet(value_buff, buff);
    write(fd, buff, sizeof(struct packet));

    close(fd);

    return 0;
}
