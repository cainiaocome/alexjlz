/*
    core.c

    last modified at: 2015/4/10
    
    defines packet and functions used in communicate between client and server
*/

#include "core.h"
#include "../utils/utils.h"
#include "../alg/alexjlz_hash.h"
#include "../log/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int alexjlz_register(char *uuid)
{
    alexjlz_log("client <<%s>> register success!\n", uuid);
    return 0;
}

int serve ( int client_fd )
{
    int state = SERVER_STATE_MACHINE_WAIT;
    int bytes_read = 0;
    char uuid[129] = {0};
    int uuid_length = 0;
    struct packet p;  // client
    struct packet q;  // server
    long server_hash = 0;
    long client_hash = 0;

    while ( state != SERVER_STATE_MACHINE_ERROR && state != SERVER_STATE_MACHINE_SERVICE )
    {
        bytes_read = readn ( client_fd, &p, sizeof(p) );
        if ( bytes_read != sizeof(p) )
        {
            alexjlz_log("wrong number(%d) read\n", bytes_read);
            close(client_fd);
            return -1;
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
                write( client_fd, &q, sizeof(q));
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
                    alexjlz_register( uuid );
                    state = SERVER_STATE_MACHINE_SERVICE;
                }
                else
                {
                    alexjlz_log("client wrong hash!\n");
                    state = SERVER_STATE_MACHINE_ERROR;
                }
                break;

            default:
                alexjlz_hash("server is in a inconsistent state!\n");
                state = SERVER_STATE_MACHINE_ERROR;
                break;
        } //switch
    } //while

    return state;
}
