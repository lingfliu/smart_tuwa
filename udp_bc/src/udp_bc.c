#include "udp_bc.h"

int main(int argn, char* argv[]){
	/********************************************
	  1. udp bc thread
	 ********************************************/

	struct sockaddr_in bcaddr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1){
		return -1;
	}

	int bcEnabled = 1;
	retval = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*) &bcEnabled, sizeof(bcEnabled));
 
    memset(&bcaddr, 0, sizeof(bcaddr));
    bcaddr.sin_family = AF_INET;
	bcaddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    bcaddr.sin_port = htons(BC_PORT);

	if(inet_pton(AF_INET, BC_ADDR, (void*) &bcaddr.sin_addr)<= 0){
		return -1;
	}

	//initialize system periodic task
	gettimeofday(&timer, NULL);

	int pos = 0;
	char* content = "TWRT-IP-BROADCAST";
	int ret = 0;

	while(1){
		sleep(3);

		ret = sendto(sock, content, strlen(content), 0, (struct sockaddr*) &bcaddr, sizeof(bcaddr));

		if (ret<0){
			printf("broadcast failed\n");
		}
		else {
			printf("broadcast send %d bytes\n", ret);
		}

		/*
		while(pos < 4){

			ret = send(sock, content+pos, 4-pos, MSG_NOSIGNAL);
			if (ret == 4 - pos){
				printf("Send broadcast\n");
				pos = 0;
			}
			else if (ret <= 0){
				printf("Send broadcast failed\n");
				pos = 0;
			}
			else {
				printf("Send partial\n");
				pos += ret;
			}
		}
		*/
	}
}


