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
    char info[512];
    char cmd[512];
};

struct packet
{
    uint32_t type;
    uint32_t flag;
    char value[1024];
};

int alexjlz_register(struct client *c);

void *serve_client( void *arg);
int ask_for_service( int server_fd );
void *serve_alexjlz( void *arg);

/*
    packet struct design:
    the most significant four bits represent packet type
*/
#define packet_type_pre (1<<31)
#define packet_type_cur (1<<30)
#define packet_type_aft (1<<29)
#define packet_type_ext (1<<28)

#define packet_register                 ( packet_type_pre | 0x1 )
#define packet_task_sign                     ( packet_type_pre | 0x2 )

#endif
