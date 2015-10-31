#include "simple_inet.h"

int inet_client_config(char* ip, int port, int proc_type, inet* client){
    client->proc = proc_type;

    switch(client->proc){
	case INET_PROC_TCP:
	    if((client->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
			return -1;
	    }
	    break;
	case INET_PROC_UDP:
	    if((client->fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
			return -1;
	    }
	    break;
		//if unknown proc, return error
	default:
	    return -1;
    }

    int flags = fcntl(client->fd, F_GETFL, 0);
    fcntl(client->fd, F_SETFL, flags | O_NONBLOCK);//set the socket as non-block 
    
    memset(&client->ip_addr, 0, sizeof(client->ip_addr));
    client->ip_addr.sin_family = AF_INET;
    client->ip_addr.sin_port = htons(port);
    if(inet_pton(AF_INET, ip, (void*)&client->ip_addr.sin_addr)<= 0){
		return -1;
    }


    return 0;
}

int inet_client_connect(inet *client){
    return connect(client->fd, (struct sockaddr*) &client->ip_addr, sizeof(client->ip_addr)); 
}

int inet_client_close(inet* client){
	if (close(client->fd) < 0 ){
		return -1;
	}
	else {
		//reset the socket fd
		switch(client->proc){
			case INET_PROC_TCP:
				if((client->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
					return -1;
				}
				break;
			case INET_PROC_UDP:
				if((client->fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
					return -1;
				}
				break;
			default:
				return -1;
		}
		int flags = fcntl(client->fd, F_GETFL, 0);
		fcntl(client->fd, F_SETFL, flags | O_NONBLOCK);//set the socket as non-block 
		return 0;
	}
}

/*************************server part******************************************/
int inet_server_config(int port, int proc_type, inet* server){
	server->proc = proc_type;

    switch(server->proc){
	case INET_PROC_TCP:
	    if((server->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
			printf("socket creation failed, ");
			switch (errno) {
				case EACCES:
					printf("EACCES\n");
					break;
				case EAFNOSUPPORT:
					printf("EAFNOSUPPORT\n");
					break;
				case EINVAL:
					printf("EINVAL\n");
					break;
				case EMFILE:
					printf("EMFILE\n");
					break;
				case ENOBUFS:
					printf("ENOBUFS\n");
					break;
				case EPROTONOSUPPORT:
					printf("EPROTONOSUPPORT\n");
					break;
			}
			return -1;
	    }
	    break;
	case INET_PROC_UDP:
	    if((server->fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
			return -1;
	    }
	    break;
		//if unknown proc, return error
	default:
	    return -1;
    }

    int flags = fcntl(server->fd, F_GETFL, 0);
    fcntl(server->fd, F_SETFL, flags | O_NONBLOCK);//set the socket as non-block 

	server->ip_addr.sin_family = AF_INET;
	server->ip_addr.sin_addr.s_addr = INADDR_ANY;
    server->ip_addr.sin_port = htons(port);

	//bind fd and ip_addr
	if( bind(server->fd, (struct sockaddr *) &(server->ip_addr), sizeof(server->ip_addr)) < 0){
		return -1;
	}

	return 0;
}

/***********close current socket and reset the server socket********************/
int inet_server_close(inet* server){
	int retval = 0;

	if (close(server->fd) < 0){
		retval = -1;
	}
	else {
		//reset socket fd
		switch(server->proc){
			case INET_PROC_TCP:
				if ( (server->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
					retval = -1;
				}
				break;
			case INET_PROC_UDP:
				if ( (server->fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
					retval = -1;
				}
				break;
			default:
				retval = -1;
		}

		int flags = fcntl(server->fd, F_GETFL, 0);
		fcntl(server->fd, F_SETFL, flags | O_NONBLOCK);//set the socket as non-block 

		//bind local addr with port
		if( bind(server->fd, (struct sockaddr *) &(server->ip_addr), sizeof(server->ip_addr)) < 0){
			retval = -1;
		}
		retval = 0;
	}

	return retval;
}
