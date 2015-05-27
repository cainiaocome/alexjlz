/*
    core.c

    last modified at: 2015/4/25
    error
    
    defines packet and functions used in communicate between client and server
*/

#include "core.h"
#include "../utils/utils.h"
#include "../alg/alexjlz_hash.h"
#include "../log/log.h"
#include "../tcpip/tcpip.h"
#include "../adt/list.h"

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

list_p client_list = NULL;

struct packet *make_packet(unsigned long t, unsigned long f, char *v, struct packet *p)
{
    bzero(p, sizeof(p));
    p->type = t;
    p->flag = f;
    strcpy((char*)&(p->value), (const char*)v);
    
    return p;
}

struct packet *parse_packet(struct packet *p)
{
    unsigned long type = p->type;
    unsigned long flag = p->flag;
    char *buff = (char *)&(p->value);

    printf("packet type: %lu\n", type);
    printf("packet flag: %lu\n", flag);
    printf("packet value:%s\n", buff);

    return p;
}

int certify_register_packet(struct packet* p)
{
    char random_str[32] = {0};
    char client_hash[32] = {0};
    char server_hash[32] = {0};

    parse_string((p->value), random_str, "random_str");
    parse_string((p->value), client_hash, "hash");
    alexjlz_hash(random_str, server_hash);

    if ( strcmp(client_hash, server_hash) == 0 )
        return 1;
    else
        return 0;
}

int alexjlz_register(struct client *c)
{
    //list_add(client_list, c, sizeof(struct client)); 
    alexjlz_log("client <<%s>> (ip:%s)  register success!\n", c->uuid, c->ip);
    return 0;
}

void *serve ( void *arg )
{
    int client_fd = *((int *)arg);
    fd_set client_fd_set;
    struct timeval tv;
    int retval;
    FD_ZERO(&client_fd_set);
    FD_SET(client_fd, &client_fd_set);
    tv.tv_sec = 6;
    tv.tv_usec = 0;

    int bytes_read = 0;
    int bytes_write = 0;
    struct packet p;  // client
    struct packet q;  // server
    struct client c;
    char *client_ip = c.ip;

    /* get remote ip address and record it */
    if ( get_remote_ip(client_fd, client_ip) == -1 )
    {
        alexjlz_log("Error, failed to get client ip, exit...\n");
        return ;
    }
    else
    {
        alexjlz_log("got connected from client %s\n", client_ip);
    }

    /* wait for client to send packet for 6 secs */
    retval = select(client_fd+1, &client_fd_set, NULL, NULL, &tv);
    if ( retval == -1 )  // error
    {
        alexjlz_log("Error, select retval == -1\n");
        close(client_fd);
        return ;
    }
    else if ( retval == 0 ) // timeout
    {
        alexjlz_log("Error, select timeout\n");
        close(client_fd);
        return;
    }
    bytes_read = readn ( client_fd, &p, sizeof(p) );  // else readable
    if ( bytes_read != sizeof(p) )
    {
        alexjlz_log("Error, serve readn read %d bytes\n", bytes_read);
        if ( check_fd(client_fd) )
            close(client_fd);
        return ;
    }
    if ( (p.type != packet_register) || !certify_register_packet(&p) )
    {
        alexjlz_log("Error, client's first packet is not of type register\n");
        close(client_fd);
        return ;
    }
    parse_string(p.value, c.uuid, "uuid");
    alexjlz_register(&c);

    if ( check_fd(client_fd) )
        close(client_fd);
    return ;
}

int ask_for_service( int server_fd )
{
    int bytes_read = 0;
    int bytes_write = 0;
    struct packet p; // client (self)
    struct packet q; // server (remote)
    char random_str[32] = {0};
    char hash[32] = {0};

    randomstr(random_str, 20);
    alexjlz_hash(random_str, hash);
    sprintf(p.value, "random_str:%s hash:%s", random_str, hash);
    p.type = packet_register;
    if (writen(server_fd, &p, sizeof(p)) != sizeof(p) )
    {
        fprintf(stderr, "Error: writen\n");
        close(server_fd);
        return -1;
    }
    
    return 0;
}

/*
int send_output(FILE *output, int server_fd)
{
    struct packet p; // client (self)
    struct packet q; // server (remote)
    int bytes_read = 0;
    int bytes_write = 0;

    while ( !feof(output) && !ferror(output) && (output!=NULL) )
    {
        bzero(&p, sizeof(p));
        p.type = packet_cur_response_continue;
        fread(p.value, 1, 1023, output);
        
        bytes_write = writen(server_fd, &p, sizeof(p));
        if ( bytes_write != sizeof(p) )
            break;
    }
    make_cur_end_packet(&p);
    bytes_write = writen(server_fd, &p, sizeof(p));
        
    return 0;
}
*/
