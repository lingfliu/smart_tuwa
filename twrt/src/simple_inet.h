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

int inet_client_config(char* ip, int port, int proc_type, inet* client);
int inet_client_connect(inet* client);//for a long connection
int inet_client_close(inet *client);

int inet_server_config(int port, int proct_type, inet* server);
int inet_server_close(inet* server);
#endif
