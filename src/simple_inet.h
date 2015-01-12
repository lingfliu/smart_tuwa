#ifndef _SIMPLE_INET_
#define _SIMPLE_INET_

#define INET_PROTOCOL_TCP 1
#define INET_PROTOCOL_UDP 2
#define INET_PROTOCOL_HTTP 3


typedef struct{
    char* ip;
    int port;
    int protocol_type;
    int connection_type;
}inet_config;

#endif
