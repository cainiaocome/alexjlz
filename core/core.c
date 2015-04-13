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

struct packet *make_pre_register_packet(char *value, struct packet *packet_buff)
{
    bzero(packet_buff, sizeof(struct packet));
    packet_buff->type = packet_pre_register;
    strncpy(packet_buff->value, value, strlen(value));

    return packet_buff;
}

struct packet *make_pre_response_packet(char *value, struct packet *packet_buff)
{
    bzero(packet_buff, sizeof(struct packet));
    packet_buff->type = packet_pre_response;
    strncpy(packet_buff->value, value, strlen(value));

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
    alexjlz_log("client %s register success\n", uuid);
    return 0;
}

int challenge_client(int client_fd)
{
    int n = 0;
    unsigned long local_hash = 0;
    unsigned long remote_hash = 0;
    char time[1024] = {0};
    char time_buff[1024] = {0};
    char uuid_buff[129] = {0};
    char chanllenge_str_buff[129] = {0};
    struct packet *packet_buff = (struct packet*)malloc(sizeof(struct packet)); 

    bzero(time_buff, sizeof(time_buff));
    bzero(packet_buff, sizeof(*packet_buff));

    if( alexjlz_time(time) == NULL )
    {
        close(client_fd);
        free(packet_buff);
        perror("alexjlz_time");
        return -1;
    }
    n = read(client_fd, packet_buff, sizeof(struct packet));
    if ( !(packet_buff->type & packet_type_pre) )
    {
        free(packet_buff);
        close(client_fd);
        return -1;
    }
    alexjlz_log("client uuid(pre cpy): %s\n", packet_buff->value);
    strncpy(uuid_buff, packet_buff->value, 128);
    alexjlz_log("client uuid(pre auth): %s\n", uuid_buff);
    
    randomstr(chanllenge_str_buff, 128);
    local_hash = alexjlz_hash(chanllenge_str_buff);
    alexjlz_log("local hash: %lu\n", local_hash);

    make_packet(packet_pre_chanllenge, 1024, chanllenge_str_buff, packet_buff);
    write(client_fd, packet_buff, sizeof(*packet_buff));

    read(client_fd, packet_buff, sizeof(*packet_buff));
    remote_hash = atol(packet_buff->value);
    alexjlz_log("remote hash: %lu\n", remote_hash);

    if(local_hash == remote_hash)
    {
        alexjlz_register(uuid_buff);
    }
    
    free(packet_buff);
    close(client_fd);
    return 0;
}
