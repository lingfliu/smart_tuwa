#include "simple_inet.h"

int inet_client_config(char* ip, int port, int proc_type, struct_serial* inet_client){
    inet_client->proc = cfg->inet_client_proc;

    switch(inet_client->proc){
	case INET_PROC_TCP:
	    if(inet_client->fd = socket(AF_INET, SOCK_STREAM, 0) < 0){
		perror("create socket failed\n");
		return -1;
	    }
	    break;
	case INET_PROC_UDP:
	    if(inet_client->fd = socket(AF_INET, SOCK_DGRAM, 0) < 0){
		perror("create socket failed\n");
		return -1;
	    }
	    break;
	default:
	    perror("Unknown proctocol\n");
	    return -1;
    }

    int flags = fcntl(inet_client->fd, F_GETL, 0);
    fcntl(inet_client->fd, F_SETL, flags | O_NONBLOCK);//set the socket as non-block 
    
    memset(inet_client->ip_addr, 0, sizeof(inet_client->ip_addr));
    inet_client->ip_addr.sin_family = AF_INET;
    inet_client->ip_addr.sin_port = htons(port);
    if(inet_pton(AF_INET, ip, inet_client->ip_addr.sin_addr)<= 0){
	perror("ip error\n");
	return -1;
    }
    return 0;
}

int inet_client_connect(struct_inet *inet_client){
    int result;
    result = connect(inet_client->fd, (struct sockaddr*) &inet_client->ip_addr, sizeof(inet_client->ip_addr)); 
    return result;
}

int inet_client_close(struct_inet* inet_client){
    return close(inet_client->fd);
}
