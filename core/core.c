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
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

//list_p client_list = create_list();
struct packet *make_packet(unsigned long t, unsigned long l, char *v, struct packet *p)
{
    if (l > 1024)
    {
        perror("make_packet: packet length too long");
        exit(-1);
    }

    bzero(p, sizeof(p));
    p->type = t;
    p->length = l;
    strncpy((char*)&(p->value), (const char*)v, l);
    
    return p;
}

struct packet *make_register_packet(struct packet *packet_buff)
{
    char *uuid = "i love freedom so much!";

    bzero(packet_buff, sizeof(struct packet));
    packet_buff->type = packet_pre_register;
    strncpy((char *)&(packet_buff->value), uuid, strlen(uuid));

    return packet_buff;
}

long make_challenge_packet(struct packet *packet_buff)
{
    char chanllenge_string[129] = {0};
    long hash = 0;

    randomstr(chanllenge_string, 128);
    hash = alexjlz_hash(chanllenge_string);

    packet_buff->type = packet_pre_chanllenge;
    strncpy((char *)&(packet_buff->value), chanllenge_string, 128);

    return hash;
}

struct packet *make_response_packet(struct packet *packet_buff)
{
    struct packet tmp;
    long hash = 0;

    memcpy(&tmp, packet_buff, sizeof(tmp));
    bzero(packet_buff, sizeof(struct packet));

    hash = alexjlz_hash((char *)&(tmp.value));

    packet_buff->type = packet_pre_response;
    packet_buff->length = 4;
    snprintf((char *)&(packet_buff->value), 1024, "%lu", hash);

    return packet_buff;
}

struct packet *make_cur_close_packet(struct packet *packet_buff)
{
    struct packet *p = packet_buff;
    bzero(p, sizeof(*p));
    p->type = packet_cur_close;

    return p;
}

struct packet *make_cur_end_packet(struct packet *packet_buff)
{
    struct packet *p = packet_buff;
    bzero(p, sizeof(*p));
    p->type = packet_cur_response_end;

    return p;
}    

struct packet *parse_packet(struct packet *p)
{
    unsigned long type = p->type;
    unsigned long length = p->length;
    char *buff = (char *)&(p->value);

    printf("packet type: %lu\n", type);
    printf("packet length: %lu\n", length);
    printf("packet value:%s\n", buff);

    return p;
}

int alexjlz_register(struct client *c)
{
    //list_add(client_list, c, sizeof(struct client)); 
    alexjlz_log("client <<%s>> register success!\n", c->uuid);
    return 0;
}

int close_service(int client_fd)
{
    struct packet p; // 
    int bytes_write = 0;
    int status = 0;

    bzero(&p, sizeof(p));
    make_cur_close_packet(&p);

    bytes_write = writen(client_fd, &p, sizeof(p));
    if ( bytes_write != sizeof(p))
    {
        alexjlz_log("close_service writen");
        status = -1;
    }

    return status;
}

void *serve ( void *arg )
{
    int client_fd = *((int *)arg);
    int state = SERVER_STATE_MACHINE_WAIT;
    int bytes_read = 0;
    int bytes_write = 0;
    char uuid[129] = {0};
    int uuid_length = 0;
    struct packet p;  // client
    struct packet q;  // server
    long server_hash = 0;
    long client_hash = 0;
    char client_ip[1024];

    if ( get_remote_ip(client_fd, client_ip) == -1 )
    {
        alexjlz_log("Error, exit...\n");
        exit(-1);
    }
    else
    {
        alexjlz_log("got connected from client %s\n", client_ip);
    }

    while ( state != SERVER_STATE_MACHINE_ERROR && state != SERVER_STATE_MACHINE_CLOSE_DONE )
    {
        if ( state != SERVER_STATE_MACHINE_SERVICE )
        {
            bytes_read = readn ( client_fd, &p, sizeof(p) );
            if ( bytes_read != sizeof(p) )
            {
                alexjlz_log("serve readn read %d bytes\n", bytes_read);
                close(client_fd);
                return ;
            }
        }

        switch ( state )
        {
            case SERVER_STATE_MACHINE_WAIT: 
                if ( p.type != packet_pre_register )
                {
                    state = SERVER_STATE_MACHINE_ERROR;
                    alexjlz_log("when server machine state is SERVER_STATE_MACHINE_WAIT, client send a wrong type: %lu\n", p.type);
                    break;
                }

                strncpy((char *)uuid, (char *)&(p.value), 128);
                uuid_length = strlen(uuid);
                alexjlz_log("uuid length:%d.\n", uuid_length);

                server_hash = make_challenge_packet(&q);
                bytes_write = writen( client_fd, &q, sizeof(q));
                if(bytes_write != sizeof(q))
                {
                    state = SERVER_STATE_MACHINE_ERROR;
                    alexjlz_log("Error while write to client\n");
                }
                state = SERVER_STATE_MACHINE_CHALLENGE;
                break;

            case SERVER_STATE_MACHINE_CHALLENGE:
                if ( p.type != packet_pre_response )
                {
                    state = SERVER_STATE_MACHINE_ERROR;
                    alexjlz_log("when server machine state is SERVER_STATE_MACHINE_CHALLENGE, client send a wrong type: %lu\n", p.type);
                    break;
                } 

                client_hash = atol((char *)&(p.value));
                if ( client_hash == server_hash )
                {
                    struct client c;
                    strncpy(c.uuid, uuid, 128);
                    alexjlz_register( &c );

                    state = SERVER_STATE_MACHINE_SERVICE;
                }
                else
                {
                    alexjlz_log("client wrong hash!\n");
                    state = SERVER_STATE_MACHINE_ERROR;
                }
                break;

            case SERVER_STATE_MACHINE_SERVICE:
            // to do
                do
                {
                    q.type = packet_cur_post;
                    strcpy(q.value, "ifconfig -a");

                    writen(client_fd, &q, sizeof(q));
                    do
                    {
                        readn(client_fd, &p, sizeof(p));
                        alexjlz_log("%s", p.value);
                    }while( p.type != packet_cur_response_end );
                }while(0);
                alexjlz_log("\n");

                close_service(client_fd);
                state = SERVER_STATE_MACHINE_CLOSE;
                break;

            case SERVER_STATE_MACHINE_CLOSE:
                make_cur_close_packet(&q);
                bytes_write = writen(client_fd, &q, sizeof(q));
                if ( bytes_write != sizeof(q) )
                {
                    perror("SERVER_STATE_MACHINE_CLOSE writen");
                    state = SERVER_STATE_MACHINE_ERROR;
                }

                state = SERVER_STATE_MACHINE_CLOSE_DONE;
                break;

            default:
                alexjlz_hash("server is in a inconsistent state!\n");
                state = SERVER_STATE_MACHINE_ERROR;
                break;
        } //switch
    } //while

    return ;
}

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

int ask_for_service( int server_fd )
{
    int state = CLIENT_STATE_MACHINE_START;
    int bytes_read = 0;
    int bytes_write = 0;
    unsigned long hash = 0;
    FILE *output = NULL;
    struct packet p; // client (self)
    struct packet q; // server (remote)

    while ( state != CLIENT_STATE_MACHINE_ERROR && state != CLIENT_STATE_MACHINE_CLOSE )
    {
        switch (state)
        {
            case CLIENT_STATE_MACHINE_START:
                make_register_packet(&p);
                
                bytes_write = writen(server_fd, &p, sizeof(p));
                if ( bytes_write != sizeof(p) )
                {
                    perror("writen");
                    state = CLIENT_STATE_MACHINE_ERROR;
                    break;
                }
                fprintf(stdout, "register packet sent!(length in bytes: %d)\n", bytes_write);
                state = CLIENT_STATE_MACHINE_REGISTER;
                break;

            case CLIENT_STATE_MACHINE_REGISTER:
                bytes_read = readn(server_fd, &q, sizeof(q));
                if ( bytes_read != sizeof(q) )
                {
                    perror("readn");
                    state = CLIENT_STATE_MACHINE_ERROR;
                }
                fprintf(stdout, "chanllenge received!\n");
                state = CLIENT_STATE_MACHINE_RESPONSE;
                break;

            case CLIENT_STATE_MACHINE_RESPONSE:
                make_response_packet(&q);
                bcopy(&q, &p, sizeof(p));
                bytes_write = writen( server_fd, &p, sizeof(p) );
                if ( bytes_write != sizeof(p) )
                {   
                    perror("writen");
                    state = CLIENT_STATE_MACHINE_ERROR;
                }
                fprintf(stdout, "response sent!\n");
                state = CLIENT_STATE_MACHINE_SERVICE;
                break;

            case CLIENT_STATE_MACHINE_SERVICE:
                do
                {
                    bytes_read = readn(server_fd, &q, sizeof(q));
                    if ( bytes_read != sizeof(q) )
                    {
                        perror("CLIENT_STATE_MACHINE_SERVICE, readn");
                        state = CLIENT_STATE_MACHINE_ERROR;
                    }
                    switch(q.type) 
                    {
                        case packet_cur_post:
                            output = get_stdout(q.value);
                            send_output(output, server_fd);
                            close_stdout(output);

                        case packet_cur_close:
                            break;

                        default:
                            q.type = packet_cur_close;
                            break;
                    }
                }while(q.type != packet_cur_close);

                state = CLIENT_STATE_MACHINE_CLOSE;
                break;

            default:
                state = CLIENT_STATE_MACHINE_CLOSE;
                break;
        }
    }
    return state;
}
