#include "simple_inet.h"

int inet_server_open(){
    return 0;
}
int on_inet_server_rx(){
    return 0;
}
int inet_server_close(){
    return 0;
}

int inet_client_connect(struct_inet *inet_client){
    int result;
    result = connect(inet_client->fd, (struct sockaddr*) &inet_client->ip_addr, sizeof(inet_client->ip_addr)); 
    return result;
}

int on_inet_client_tx(struct_inet* inet_client, char* buff, int len){
}

int inet_client_close(struct_inet* inet_client){
    return close(inet_client->fd);
}
