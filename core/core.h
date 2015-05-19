/*
    core.h

    last modified at: 2015/4/12
    
    defines packet and functions used in communicate between client and server
*/
#ifndef _CORE_H
#define _CORE_H

#include <stdio.h>

struct client
{
    char uuid[256];
    char name[256];
    char info[4096];
};

struct packet
{
    unsigned long type;
    unsigned long length;
    char value[1024];
};

struct packet *make_packet(unsigned long, unsigned long, char*, struct packet*);
struct packet *parse_packet(struct packet *p);
int alexjlz_register(struct client *c);

int close_service(int client_fd);
int serve( int client_fd );
int send_output(FILE *output, int server_fd);
int ask_for_service( int server_fd );

struct packet *make_register_packet(struct packet *packet_buff);
long make_challenge_packet(struct packet *packet_buff);
struct packet *make_response_packet(struct packet *packet_buff);
struct packet *make_cur_close_packet(struct packet *packet_buff);
struct packet *make_cur_end_packet(struct packet *packet_buff);
/*
    packet struct design:
    the most significant four bits represent packet type
*/
#define packet_type_pre (1<<31)
#define packet_type_cur (1<<30)
#define packet_type_aft (1<<29)
#define packet_type_ext (1<<28)

#define packet_pre_register                 ( packet_type_pre | 0x1 )
#define packet_pre_chanllenge               ( packet_type_pre | 0x2 )
#define packet_pre_response                 ( packet_type_pre | 0x3 )

#define packet_cur_get                      ( packet_type_cur | 0x1 )
#define packet_cur_post                     ( packet_type_cur | 0x2 )
#define packet_cur_response_begin           ( packet_type_cur | 0x3 )
#define packet_cur_response_continue        ( packet_type_cur | 0x4 )
#define packet_cur_response_end             ( packet_type_cur | 0x4 )
#define packet_cur_close                    ( packet_type_cur | 0xffffff )

/*
    server state machine
*/
#define SERVER_STATE_MACHINE ( 0x0 )
#define SERVER_STATE_MACHINE_ERROR ( 1<<31 )
#define SERVER_STATE_MACHINE_CLOSE ( 1<<30 )
#define SERVER_STATE_MACHINE_CLOSE_DONE ( 1<< 29 )
#define SERVER_STATE_MACHINE_WAIT ( 1<<1)
#define SERVER_STATE_MACHINE_CHALLENGE ( 1<<2 )
#define SERVER_STATE_MACHINE_SERVICE ( 1<<3 )

/*
    client state machine
*/
#define CLIENT_STATE_MACHINE ( 0x0 )
#define CLIENT_STATE_MACHINE_ERROR ( 1<<31 )
#define CLIENT_STATE_MACHINE_CLOSE ( 1<<30 )
#define CLIENT_STATE_MACHINE_START ( 1<<1 )
#define CLIENT_STATE_MACHINE_REGISTER ( 1<<2 )
#define CLIENT_STATE_MACHINE_RESPONSE ( 1<<3 )
#define CLIENT_STATE_MACHINE_SERVICE  ( 1<<4 )

#endif
