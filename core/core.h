/*
    core.h

    last modified at: 2015/4/12
    
    defines packet and functions used in communicate between client and server
*/
#ifndef _CORE_H
#define _CORE_H

struct packet
{
    unsigned long type;
    unsigned long length;
    char value[1024];
};

struct packet *make_packet(unsigned long, unsigned long, char*, struct packet*);
struct packet *parse_packet(struct packet *p);
int alexjlz_register(char *uuid);

struct packet *make_register_packet(struct packet *packet_buff);
long make_challenge_packet(struct packet *packet_buff);
struct packet *make_response_packet(struct packet *packet_buff);
/*
    packet struct design:
    the most significant four bits represent packet type
*/
#define packet_type_pre (1<<31)
#define packet_type_cur (1<<30)
#define packet_type_aft (1<<29)
#define packet_type_ext (1<<28)

#define packet_pre_register         ( packet_type_pre | 0x1 )
#define packet_pre_chanllenge       ( packet_type_pre | 0x2 )
#define packet_pre_response         ( packet_type_pre | 0x3 )

/*
    server state machine
*/
#define SERVER_STATE_MACHINE ( 0x0 )
#define SERVER_STATE_MACHINE_ERROR ( 1<<31 )
#define SERVER_STATE_MACHINE_WAIT ( 1<<1)
#define SERVER_STATE_MACHINE_CHALLENGE ( 1<<2 )
#define SERVER_STATE_MACHINE_SERVICE ( 1<<3 )

#endif
