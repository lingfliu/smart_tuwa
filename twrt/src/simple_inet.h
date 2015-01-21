#ifndef _SIMPLE_INET_
#define _SIMPLE_INET_

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define INET_PROC_TCP 1
#define INET_PROC_UDP 2
#define INET_BUFF_LEN 255
int inet_server_listen();
int on_inet_server_rx();
int inet_server_close();
int inet_client_connect();
int on_inet_client_rx();
int inet_client_close();

#endif
