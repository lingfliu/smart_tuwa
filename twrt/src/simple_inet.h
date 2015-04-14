#ifndef _SIMPLE_INET_
#define _SIMPLE_INET_

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define INET_PROC_TCP 1
#define INET_PROC_UDP 2

#define INET_BUFF_LEN 255


typedef struct{
    int type;
    int proc;   
    int fd;
    struct sockaddr_in ip_addr;
}inet;

//int inet_server_config(int port, int proc_type, struct_serial* inet_server);
//int inet_server_open(struct_inet* inet_server);
//int on_inet_server_rx(struct_inet* inet_server, char* buff);
//int on_inet_server_tx(struct_inet* inet_client, char* buff, int len);
//int on_inet_server_tx_ack(struct_inet* inet_client, char* bytes, int len, char* ack);
//int inet_server_close(struct_inet* inet_server);

int inet_client_config(char* ip, int port, int proc_type, inet* client);
int inet_client_connect(inet* client);//for a long connection
int inet_client_close(inet *client);
#endif
