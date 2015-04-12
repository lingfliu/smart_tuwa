#include "simple_inet.h"

int inet_client_config(char* ip, int port, int proc_type, inet* client){
    client->proc = proc_type;

    switch(client->proc){
	case INET_PROC_TCP:
	    if(client->fd = socket(AF_INET, SOCK_STREAM, 0) < 0){
		perror("create socket failed\n");
		return -1;
	    }
	    break;
	case INET_PROC_UDP:
	    if(client->fd = socket(AF_INET, SOCK_DGRAM, 0) < 0){
		perror("create socket failed\n");
		return -1;
	    }
	    break;
	default:
	    perror("Unknown proctocol\n");
	    return -1;
    }

    int flags = fcntl(client->fd, F_GETFL, 0);
    fcntl(client->fd, F_SETFL, flags | O_NONBLOCK);//set the socket as non-block 
    
    memset(&client->ip_addr, 0, sizeof(client->ip_addr));
    client->ip_addr.sin_family = AF_INET;
    client->ip_addr.sin_port = htons(port);
    if(inet_pton(AF_INET, ip, (void*)&client->ip_addr.sin_addr)<= 0){
	perror("ip error\n");
	return -1;
    }
    return 0;
}

int inet_client_connect(inet *client){
    return connect(client->fd, (struct sockaddr*) &client->ip_addr, sizeof(client->ip_addr)); 
}

int inet_client_close(inet* client){
    return close(client->fd);
}
