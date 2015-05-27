/*
    core.h

    last modified at: 2015/4/12
    
    defines packet and functions used in communicate between client and server
*/
#ifndef _CORE_H
#define _CORE_H

#include <stdio.h>
#include <stdint.h>

struct client
{
    char uuid[256];
    char ip[32];
    char cmd[1024];
};

struct packet
{
    uint32_t type;
    uint32_t flag;
    char value[1024];
};

struct packet *make_packet(unsigned long, unsigned long, char*, struct packet*);
struct packet *parse_packet(struct packet *p);
int alexjlz_register(struct client *c);

int close_service(int client_fd);
void *serve( void *arg);
int send_output(FILE *output, int server_fd);
int ask_for_service( int server_fd );

struct packet *make_register_packet(struct packet *packet_buff);
/*
    packet struct design:
    the most significant four bits represent packet type
*/
#define packet_type_pre (1<<31)
#define packet_type_cur (1<<30)
#define packet_type_aft (1<<29)
#define packet_type_ext (1<<28)

#define packet_register                 ( packet_type_pre | 0x1 )

#define packet_cur_get                      ( packet_type_cur | 0x1 )
#define packet_cur_post                     ( packet_type_cur | 0x2 )
#define packet_cur_close                    ( packet_type_cur | 0xffffff )

#endif
