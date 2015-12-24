#include "twrt.h"

int main(int argn, char* argv[]){
	message *msg_auth;
	message *msg_stamp;

	/********************************************
	  1. configure IO and buffers
	 ********************************************/
	get_config(&cfg);

	serial_config(cfg.serial_name, cfg.serial_type, cfg.serial_baudrate, &srl);
	buffer_ring_byte_create(&buff_serial, BUFF_RING_LEN);

	inet_client_config(cfg.server_ip, cfg.server_port, cfg.server_proc, &client);
	buffer_ring_byte_create(&buff_client, BUFF_RING_LEN);

	inet_server_config(cfg.localhost_port, cfg.localhost_proc, &localhost);

	/********************************************
	  2. initialize sys message queue (client) 
	 ********************************************/

	msg_q_rx = message_queue_create();
	message_queue_init(msg_q_rx); //save the head of the queue
	msg_q_rx_h = msg_q_rx;
	msg_q_tx = message_queue_create();
	message_queue_init(msg_q_tx);
	msg_q_tx_h = msg_q_tx;
	msg_q_tx_req = message_queue_create();
	message_queue_init(msg_q_tx_req);
	msg_q_tx_req_h = msg_q_tx_req;


	/********************************************
	  3. initialize sys message queue (client) 
	 ********************************************/
	sys_init(&sys);

	/********************************************
	  4. localhost start listening (threads)
	 ********************************************/
	if(pthread_create(&thrd_localhost, NULL, run_localhost, NULL) <0){
		return -1;
	}

	/********************************************
	  5. open serial (thread)
	 ********************************************/
	if (serial_open(&srl) < 0) {
		sys.server_status = SERIAL_OFF;
	}
	else {
		sys.server_status = SERIAL_ON;
	}

	/********************************************
	  6. initialize muts, threads and conds
	 ********************************************/

	//initialize sys_msg thread, mut, and cond
	pthread_mutex_init(&mut_msg_rx, NULL);
	pthread_mutex_init(&mut_msg_tx, NULL);

	if(pthread_create(&thrd_sys_msg_rx, NULL, run_sys_msg_rx, NULL) <0){
		return -1;
	}

	if(pthread_create(&thrd_sys_msg_tx, NULL, run_sys_msg_tx, NULL) < 0){
		return -1;
	}

	//initialize client threads, mut, and cond
	pthread_mutex_init(&mut_client, NULL);
	pthread_cond_init(&cond_client, NULL);

	if(pthread_create(&thrd_client_rx, NULL, run_client_rx, NULL) < 0){
		return -1;
	}

	//initialize serial threads, mut, and cond
	pthread_mutex_init(&mut_serial,NULL);
	pthread_cond_init(&cond_serial, NULL);

	if(pthread_create(&thrd_serial_rx, NULL, run_serial_rx, NULL) < 0){
		return -1;
	}

	//initialize system periodic task
	gettimeofday(&(sys.timer_pulse), NULL);
	gettimeofday(&(sys.timer_reset), NULL);
	gettimeofday(&(sys.timer_sync), NULL);

	if(pthread_create(&thrd_sys_ptask, NULL, run_sys_ptask, NULL) < 0){
		return -1;
	}

	//create fixed messages
	msg_auth = message_create_req_auth_gw(SYS_LEN_LIC, sys.lic, sys.id, sys.tx_msg_stamp++); //create auth gw message
	msg_stamp = message_create_req_stamp(sys.id, sys.tx_msg_stamp++); //create auth gw message

	//setups with server, run only when GW is online
	//if GW is offline, no sync will be performed 
	//but this may compromise the whole security of the system
	while(1){
		sleep(1);

		/* connect to serial*/
		if (sys.serial_status == SERIAL_OFF) {
			if (serial_open(&srl) < 0) {
				printf("serial cannot be opened, check the hardware\n");
			}
			else {
				printf("serial opened, znet connected\n");
				sys.serial_status = SERIAL_ON;
			}
		}

		/* connect to the server */
		if(sys.server_status == SERVER_DISCONNECT){
			if (inet_client_connect(&client) == -1){
				if(errno == EINPROGRESS){ //connection is in progress 
					inet_timeout.tv_sec = 2; //set timeout as in 2 seconds
					inet_timeout.tv_usec = 0;
					FD_ZERO(&inet_fds);
					FD_SET(client.fd, &inet_fds);
					retval = select(client.fd+1, NULL, &inet_fds, NULL, &inet_timeout);
					if(retval == -1 || retval == 0){ //error or timeout
						//printf("failed to connect server\n");
						inet_client_close(&client);
					}
					else {
						printf("connected to server\n");
						sys.server_status = SERVER_CONNECT;
					}
				}
				else { 
					//printf("failed to connect server\n");
					inet_client_close(&client);
				}
			}
			else {
				printf("connected to server\n");
				sys.server_status = SERVER_CONNECT;
			}
		}
		/* if server connected */
		else {
			//if license is not authed by the server, communication to server will be stopped, but the rest part (znet, localhost) will keep functioning
			if(sys.lic_status == LIC_UNKNOWN) {
				pthread_mutex_lock(&mut_msg_tx);
				if(message_queue_find_stamp(msg_q_tx_req_h, msg_auth->stamp) == 0) { //if previous auth req is not responded and is flushed
					msg_auth->stamp = sys.tx_msg_stamp++;
					msg_q_tx = message_queue_put(msg_q_tx, msg_auth);
				}
				pthread_mutex_unlock(&mut_msg_tx);
			}
			else if(sys.lic_status == LIC_INVALID) {
				return -1;
			}
			else if (sys.lic_status == LIC_VALID && sys.u_stamp < 0) {
				//this will keep running untill the stamp is synchronized 
				pthread_mutex_lock(&mut_msg_tx);
				if(message_queue_find_stamp(msg_q_tx_req, msg_stamp->stamp) == 0) {
					msg_stamp->stamp = sys.tx_msg_stamp++;
					msg_q_tx = message_queue_put(msg_q_tx, msg_stamp);
				}
				pthread_mutex_unlock(&mut_msg_tx);
			}
		}
	}
}

void *run_serial_rx(){
	pthread_detach(pthread_self());
	message *msg = message_create();
	int len;
	while(1){
		usleep(5000);

		//translate bytes
		while(bytes2message(&buff_serial, msg)>0){
			printf("received msg from znet, msg type=%d\n",msg->data_type);
			pthread_mutex_lock(&mut_msg_rx);
			msg_q_rx = message_queue_put(msg_q_rx, msg);
			pthread_mutex_unlock(&mut_msg_rx);
			message_flush(msg);
			usleep(5000);
		}

		//receive from serial
		if(sys.serial_status = SERIAL_ON) {
			len = read(srl.fd, read_serial, SERIAL_BUFF_LEN);//non-blocking reading, return immediately
			if(len>0){
				buffer_ring_byte_put(&buff_serial, read_serial, len);
				//translate bytes into message
			}
			else {
				if(errno == EAGAIN || errno == EINTR){ //reading in progress
					continue;
				}
				else{ //io error, close the serial port and re-open it in main thread
					printf("znet disconnect\n");
					sys.serial_status = SERIAL_OFF;
					serial_close(&srl);
				}
			}
		}
		/* diconnected serial, re-open in main thread*/
		else {
			continue;
		}
	}
}

void *run_serial_tx(void *arg){
	message *msg = (message*)arg;
	int len = MSG_LEN_FIXED+msg->data_len;
	int pos = 0;
	int ret;
	char *bytes = calloc(len,sizeof(char));
	message2bytes(msg, bytes);

	if(len == 0)
		pthread_exit(0);

	while (pos < len && sys.serial_status == SERIAL_ON) {
		printf("sending msg, type=%d\n",msg->data_type);
	   ret = write(srl.fd, bytes+pos, len);
	   if (ret == len-pos) {
		   //printf("message is send to znet at once\n");
		   free(bytes);
		   pthread_exit(0);
	   }
	   else if (ret < 0) {
		   if(errno == EAGAIN || errno == EINTR) {
			   usleep(5000);
			   continue;
		   }
		   else {
			   printf("serial broken\n");
			   free(bytes); //don't forget to free the mem
			   serial_close(&srl);
			   sys.serial_status = SERIAL_OFF;
			   pthread_exit(0);
		   }
	   }
	   else if (ret == 0) {
		   printf("serial broken\n");
		   free(bytes);
		   serial_close(&srl);
		   sys.serial_status = SERIAL_OFF;
		   pthread_exit(0);
	   }
	   else {
			pos += ret;
	   }
	}
	printf("message is send to znet\n");
	free(bytes); //don't forget to free the mem
	pthread_exit(0);
}

void *run_client_rx(){
	pthread_detach(pthread_self());
	message *msg = message_create();
	int len;
	while(1){
		usleep(5000);
		if(sys.server_status == SERVER_CONNECT){
			len = recv(client.fd, read_client, INET_BUFF_LEN, 0);//non-blocking reading, return immediately
			if(len>0){
				printf("received bytes len = %d\n", len);
				buffer_ring_byte_put(&buff_client, read_client, len);
				while(bytes2message(&buff_client, msg)>0){
					printf("received msg type = %d\n", msg->data_type);
					pthread_mutex_lock(&mut_msg_rx);
					msg_q_rx = message_queue_put(msg_q_rx, msg);
					pthread_mutex_unlock(&mut_msg_rx);
					message_flush(msg);
					usleep(1000);
				}
			}
			else if(len == 0){//if disconnected, reconnect
				printf("connection to server broken when receiving\n");
				on_inet_client_disconnect();
			}
			else if(len < 0) {
				if(errno == EAGAIN || errno == EINTR){
					//printf("on receiving from server\n");
					continue;
					//on_inet_client_disconnect();
					//printf("connection broken %d\n", len);
				}
				else {
					printf("connection to server broken when receiving\n");
					on_inet_client_disconnect();
				}
			}
		}
	}
}

void *run_client_tx(void *arg){
	message *msg = (message*)arg;
	int len = MSG_LEN_FIXED+msg->data_len;
	if(len == 0) {
		pthread_exit(0);
	}

	printf("sending message type=%d\n",msg->data_type);
	char *bytes = calloc(len,sizeof(char)); 
	message2bytes(msg, bytes);
	int ret; 
	int pos = 0;

	while(pos < len && sys.server_status == SERVER_CONNECT){
		ret = send(client.fd, bytes+pos, len - pos, MSG_NOSIGNAL);
		if (ret == len - pos) {
			//printf("msg is sent to server at once\n");
			free(bytes); //don't forget to free the mem
			pthread_exit(0);
		}
		else if(ret < 0) { //send failed
			if(errno == EAGAIN || errno == EINTR){ //buff is full or interrupted
				//printf("on sending to server\n");
				usleep(1000);
				continue;
			}
			else if(errno == ECONNRESET || errno == EPIPE){ //connection broke
				printf("connection to server broken\n");
				on_inet_client_disconnect();
				break;
			}
			else {
				printf("connection to server broken\n");
				on_inet_client_disconnect();
				break;
			}
		}
		else if (ret == 0) {
			printf("connection to server broken\n");
			on_inet_client_disconnect();
			break;
		}
		else { //send partial data
			pos += ret;
			continue;
		}
	}
	//message is send in multiple times
	printf("msg is sent to server\n");
	free(bytes); //don't forget to free the mem
	pthread_exit(0);
}

void on_inet_client_disconnect(){
    //when disconnected, clear reset lic status as unknown
	sys.server_status = SERVER_DISCONNECT;
    sys.lic_status = LIC_UNKNOWN;

	inet_client_close(&client); //close the connection first
	//connect the server
	/*
	if(inet_client_connect(&client) == -1){
		if(errno == EINPROGRESS){ //connection is in progress 
			inet_timeout.tv_sec = 2; //set timeout as in 2 seconds
			inet_timeout.tv_usec = 0;
			FD_ZERO(&inet_fds);
			FD_SET(client.fd, &inet_fds);
			retval = select(client.fd+1, NULL, &inet_fds, NULL, &inet_timeout);
			if(retval == -1 || retval == 0){ //error or timeout
				//printf("failed to reconnect to server\n");
				inet_client_close(&client);
				sys.server_status = SERVER_DISCONNECT;
			}else{
				//printf("reconnected to server\n");
				sys.server_status = SERVER_CONNECT;
			    return;	
			}
		}else{ //retry connecting to the server
			//printf("failed to reconnect to server\n");
			sys.server_status = SERVER_DISCONNECT;
			inet_client_close(&client);
		}
	}
	else {
		//printf("reconnected to server\n");
		sys.server_status = SERVER_CONNECT;
	}
	*/
}

//this thread only deals with serial and client rx messages
void *run_sys_msg_rx(){
	pthread_detach(pthread_self());
	message *msg = message_create();
	while(1){
		usleep(5000);
		pthread_mutex_lock(&mut_msg_rx);
		if(message_queue_getlen(msg_q_rx_h) > 0){
			msg_q_rx_h = message_queue_get(msg_q_rx_h, msg); //read from the head
			pthread_mutex_unlock(&mut_msg_rx);//unlock msg_q_rx first
			handle_msg_rx(msg);
			message_flush(msg);
		}
		else {
			pthread_mutex_unlock(&mut_msg_rx);//unlock msg_q_rx first
		}
	}
}

void *run_sys_msg_tx(){
	pthread_detach(pthread_self());
	message *msg = message_create();

	while(1){
		usleep(1000);
		pthread_mutex_lock(&mut_msg_tx);
		if(message_queue_getlen(msg_q_tx_h) == 0){//if empty, do nothing
			pthread_mutex_unlock(&mut_msg_tx);
		}else{
			msg_q_tx_h = message_queue_get(msg_q_tx_h, msg);
			if(message_isreq(msg)){ //put req msg into req queue, add timeval to the req
				msg_q_tx_req = message_queue_put(msg_q_tx_req, msg);
				gettimeofday(&(msg_q_tx_req->time), NULL);
			}
			pthread_mutex_unlock(&mut_msg_tx);

			switch(message_tx_dest(msg)){
				case MSG_TO_ZNET: 
					if( sys.serial_status == SERIAL_ON) {
						if(pthread_create(&thrd_serial_tx, NULL, run_serial_tx, (void*) msg) < 0){
							break;
						}
						else {
							pthread_join(thrd_serial_tx, NULL);
							break;
						}
					}
					else {
						break;
					}
				case MSG_TO_SERVER:
					if(sys.server_status == SERVER_CONNECT) {
						if(sys.lic_status != LIC_VALID){//if not authed, send only auth msg
							if(msg->data_type == DATA_REQ_AUTH_GW){
								if(pthread_create(&thrd_client_tx, NULL, run_client_tx, (void*) msg) < 0){
									break;
								}
								else {
									pthread_join(thrd_client_tx, NULL);
									break;
								}
							}
						}
						else {
							if(pthread_create(&thrd_client_tx, NULL, run_client_tx, (void*) msg) < 0){
								break;
							}
							else {
								pthread_join(thrd_client_tx, NULL);
								break;
							}
						}
					}
					else {
						break;
					}
				default://if unknown, flush the message from the queue
					break;
			}
			message_flush(msg);
		}
	}
}


void* run_localhost(){
	pthread_detach(pthread_self());

	struct sockaddr_in localuser_sock;
	socklen_t len = sizeof(struct sockaddr_in);
	int skt;
	localuser *usr;

	int res  = listen(localhost.fd, 10) ;
	printf("listening result = %d\n", res);

	if (res == -1) {
		printf("failed to start listening\n");
		switch (errno) {
			case EADDRINUSE:
				printf("addr inuse\n");
				break;
			case EBADF:
				printf("invalid descriptor\n");
				break;
			case ENOTSOCK:
				printf("not a sock\n");
				break;
			case EOPNOTSUPP:
				printf("not supported\n");
				break;
		}
		pthread_exit(0);
	}


	printf("localhost listening \n");
	while(1) {
		usleep(5000);
		skt = accept(localhost.fd, (struct sockaddr *) &localuser_sock, &len);
		if (skt <= 0){
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				usleep(5000);
				continue;
			}
			else {
				pthread_exit(0);
			}
		}
		else {
			printf("localuser connected");
			usr = sys_localuser_login(&sys, skt);
		    gettimeofday(&(usr->time_lastactive), NULL);
			pthread_create( &(usr->thrd_rx), NULL, run_localuser_rx, usr);
		}
	}
}

void* run_localuser_rx(void* arg) {
	localuser* usr = (localuser*) arg;
	message *msg = message_create();
	message *msg_tx;
	struct timeval timer;
	localbundle bundle;
	bundle.usr = usr;
	int len;
	char *tx_result;
	while(1) {
		usleep(5000);
		gettimeofday(&timer, NULL);
		len = recv(usr->skt, usr->buff_io, BUFF_IO_LEN, 0);
		if(len>0){
			//printf("received data from user %s: %s\n", usr->id, usr->buff_io);
			buffer_ring_byte_put(&(usr->buff), usr->buff_io, len);
			if( bytes2message(&(usr->buff), msg) > 0 ) {
				printf("received msg from user %s: dev type = %d data type = %d\n", usr->id, msg->dev_type, msg->data_type);
				//if usr not authed, wait until req auth is received
				if ( !usr->is_authed ) {
					if (msg->data_type == DATA_REQ_AUTH_LOCAL) {
						if ( !memcmp(msg->data, DEFAULT_AUTHCODE, 8) && !memcmp(msg->gateway_id, sys.id, MSG_LEN_ID_GW) ) {
							//register the user
							memcpy(usr->id, msg->dev_id, MSG_LEN_ID_DEV); //dev_is is the user id (app phone id)
							usr->is_authed = 1;
							printf("user authed to the system\n");

							//send back ack message
							msg = message_create_ack_auth_local(sys.id, msg->dev_id, msg->dev_type, AUTH_OK);

							bundle.msg = msg;

							pthread_create( &(usr->thrd_tx), NULL, run_localuser_tx, &bundle);
							pthread_join(usr->thrd_tx, NULL);
							//don't forget to flush the message
							message_flush(msg);

							gettimeofday( &(usr->time_lastactive), NULL );
						}
						//otherwise close the socket
						else {
							msg = message_create_ack_auth_local(sys.id, msg->dev_id, msg->dev_type, AUTH_NO);
							bundle.msg = msg;
							pthread_create( &(usr->thrd_tx), NULL, run_localuser_tx, &bundle);
							pthread_join(usr->thrd_tx, NULL);
							//don't forget to flush the message
							message_flush(msg);

							if (tx_result != NULL){
								free(tx_result);
							}

							localuser_delete(usr);
							pthread_exit(0);
						}
					}
					//a prelogined socket should always send auth as the first message
					else {
						//check the timeout
						if (timediff_s(usr->time_lastactive, timer) > DEFAULT_LOCALHOST_TIMEOUT) {
							localuser_delete(usr);
							pthread_exit(0);
						} 
						else {
							continue;
						}
					}
				}
				//if user is authed
				else {
					printf("handle local message, msg type = %d\n", msg->data_type);
					handle_local_message(msg, usr);
					message_flush(msg);
				}
			}
			else {
				//check the timeout
				if (timediff_s(usr->time_lastactive, timer) > DEFAULT_LOCALHOST_TIMEOUT) {
					localuser_delete(usr);
					pthread_exit(0);
				}
				else {
					continue;
				}
			}
		}
		else if(len == 0){//if disconnected, close the socket
			localuser_delete(usr);
			pthread_exit(0);
		}
		else if(errno == EAGAIN || errno == EINTR ){
			//check the timeout
			if (timediff_s(usr->time_lastactive, timer) > DEFAULT_LOCALHOST_TIMEOUT) {
				localuser_delete(usr);
				pthread_exit(0);
			}
			else {
				continue;
			}
		}
	}
}

void* run_localuser_tx(void *arg){
	localbundle *bundle = (localbundle*) arg;
	localuser *usr = bundle->usr;
	message *msg = bundle->msg;
	char *local_status = &(usr->tx_status);

	int len = MSG_LEN_FIXED+msg->data_len;
	if(len == 0) {
		*local_status = LOCAL_STATUS_MSGINVALID;
		pthread_exit((void*) local_status);
	}

	char *bytes = calloc(len,sizeof(char)); 
	message2bytes(msg, bytes);
	int ret; 
	int pos = 0;

	while(pos < len) {
		ret = send( usr->skt, bytes+pos, len - pos, 0 );
		if( ret == len - pos ) {
			free(bytes); //don't forget to free the mem
			*local_status = LOCAL_STATUS_EXITNORMAL;
			pthread_exit((void*) local_status);
		}

		if(ret == -1) { //send failed
			if( errno == EAGAIN || errno == EINTR ) { //buff is full or interrupted
				usleep(1000);
				continue;
			}
			if(errno == ECONNRESET) { //connection broke
				free(bytes);
				*local_status = LOCAL_STATUS_SKTDISCONNECT;
				pthread_exit((void*) local_status);
			}
			free(bytes); //don't forget to free the mem
			*local_status = LOCAL_STATUS_SKTDISCONNECT;
			pthread_exit((void*) local_status);
		}
		else { //send partial data
			pos += ret;
		}
	}
	free(bytes); //don't forget to free the mem
	*local_status = LOCAL_STATUS_EXITNORMAL;
	pthread_exit( (void*) local_status );
}

void* run_sys_ptask(){
	pthread_detach(pthread_self());

    struct timeval timer;
    message* msg;
    int m;

	while(1){	
		//periodic tasks
		sleep(1);
		gettimeofday(&timer, NULL);

		//req msg cleaning
		pthread_mutex_lock(&mut_msg_tx);

		msg = message_create();
		//if tx_req is not empty and messages are outdated
		while(message_queue_getlen(msg_q_tx_req_h) > 0 && timediff_ms(msg_q_tx_req_h->time, timer)>=TIMER_REQ) {
			msg_q_tx_req_h = message_queue_get(msg_q_tx_req_h, msg); //remove the un-responed msg
			message_flush(msg);
		}
		pthread_mutex_unlock(&mut_msg_tx);

		//sync the gw and znet
		if(timediff_s(sys.timer_sync, timer)>TIMER_SYNC){
			for(m = 0; m<ZNET_SIZE; m++){//synchronize the znodes
				if(!znode_isempty(&(sys.znode_list[m]))){
					if (msg != NULL){
						message_destroy(msg); //destroy old message before cerating one
					}
					msg = message_create_sync(sys.znode_list[m].status_len, sys.znode_list[m].status, sys.znode_list[m].u_stamp, sys.id, sys.znode_list[m].id, sys.znode_list[m].type);
					pthread_mutex_lock(&mut_msg_tx);
					msg_q_tx = message_queue_put(msg_q_tx, msg);//send the hb to the server
					pthread_mutex_unlock(&mut_msg_tx);
				}
			}

			//synchronize the root
			//msg = message_create_sync(SYS_LEN_STATUS, sys.status, sys.u_stamp, sys.id, NULL_DEV, 0, 0);
			//pthread_mutex_lock(&mut_msg_tx);
			//msg_q_tx = message_queue_put(msg_q_tx, msg);//send the hb to the server
			//pthread_mutex_unlock(&mut_msg_tx);

			//reset the timer
			gettimeofday(&(sys.timer_sync), NULL);
		}
		

		//tcp pulse
		
		if(timediff_ms(sys.timer_pulse, timer)>TIMER_PULSE){
			if (msg != NULL) {
				message_destroy(msg); //destroy old message before cerating one
			}
			msg = message_create_pulse(sys.id);
			pthread_mutex_lock(&mut_msg_tx);
			msg_q_tx = message_queue_put(msg_q_tx, msg);//send the hb to the server
			gettimeofday(&(sys.timer_pulse), NULL); //update the timer
			pthread_mutex_unlock(&mut_msg_tx);

			//reset the timer
			gettimeofday(&(sys.timer_pulse), NULL);
		}

		if(msg != NULL){
			message_destroy(msg); //memory cleanup
		}

	}
}

//message handle function
int handle_msg_rx(message *msg){
	message *msg_tx;
	int idx;
	int result = 0;
	int val;
	long vall;
	int m;
	localbundle bundle;
	localuser *usr;
	char* local_tx_result;

	int len_descrip;
	char descrip[128];

	if(!message_isvalid(msg))
		return -1;

	switch(msg->data_type){
		case DATA_STAT: 
			//update stat
			idx = sys_znode_update(&sys, msg);

			printf("received data stat from znet, dev index = %d\n", idx);

			//if update valid, send synchronization to server
			if(idx >= 0) {
				msg_tx = message_create_sync(sys.znode_list[idx].status_len, sys.znode_list[idx].status, sys.znode_list[idx].u_stamp, sys.id, sys.znode_list[idx].id, sys.znode_list[idx].type);
				pthread_mutex_lock(&mut_msg_tx);
				msg_q_tx = message_queue_put(msg_q_tx, msg_tx);
				pthread_mutex_unlock(&mut_msg_tx);
				message_destroy(msg_tx);
				result = 0;

				//send stat message to localuser if stat msg is valid
				if(sys_get_localuser_num(&sys) > 0) {
					//send the status to local-users
					for( m = 0; m < LOCALUSER_SIZE; m ++) {
						usr = &(sys.localuser_list[m]);
						if ( !localuser_isnull(usr) ) {
							bundle.usr = usr;
							bundle.msg = msg;
							pthread_create( &(usr->thrd_tx), NULL, run_localuser_tx, &bundle);
							pthread_join(usr->thrd_tx, (void**) &local_tx_result);
							//every tx thread should be checked for the connection
							if (*local_tx_result == LOCAL_STATUS_SKTDISCONNECT) {
								localuser_delete(usr);
							}
							else {
								//update the time_lastactive
								gettimeofday( &(usr->time_lastactive), NULL );
							}
						}
					}
				}
			}
			break;

		case DATA_CTRL:
			printf("received data ctrl from server, len = %d\n", msg->data_len);
			if (msg->dev_type == DEV_GW_FAN){
				/*
				 * handle controlling of gw fan
				 */
				printf("received gw fan ctrl = %s \n", msg->data);
				if (*(msg->data) == CTRL_ON){
					fan_control(1);
					sys.fan_status = STAT_ON;
				}
				else if (*(msg->data) == CTRL_OFF){
					fan_control(0);
					sys.fan_status = STAT_OFF;
				}
				else {
					//do nothing
				}
			}
			else {
				//send ctrl to znet
				msg_tx = message_create_ctrl(msg->data_len, msg->data, msg->gateway_id, msg->dev_id, msg->dev_type);
				pthread_mutex_lock(&mut_msg_tx);
				msg_q_tx = message_queue_put(msg_q_tx, msg_tx);
				pthread_mutex_unlock(&mut_msg_tx);
				message_destroy(msg_tx);
				result = 0;
			}
			break;

		case DATA_REQ_SYNC:
			printf("received data sync\n");
			if (msg->dev_type == DEV_GW_FAN){
				/*
				 * sending back fan status
				 */
				printf("received status req for GW fan, fan status = %s\n", &(sys.fan_status));
				msg_tx = message_create_stat(1, &(sys.fan_status), sys.id, NULL_DEV, DEV_GW_FAN); 

				pthread_mutex_lock(&mut_msg_tx);
				msg_q_tx = message_queue_put(msg_q_tx, msg_tx);
				pthread_mutex_unlock(&mut_msg_tx);
				message_destroy(msg_tx);
				result = 0;
				break;
			}
			else {
				msg_tx = sys_sync(&sys, msg);
				if(msg_tx == NULL) { //if server status is newer than local
					result = 0;
					break;
				} 
				else {
					pthread_mutex_lock(&mut_msg_tx);
					msg_q_tx = message_queue_put(msg_q_tx, msg_tx);
					pthread_mutex_unlock(&mut_msg_tx);
					message_destroy(msg_tx);
					result = 0;
					break;
				}
			}

		case DATA_ACK_AUTH_GW:
			printf("received data auth\n");
			pthread_mutex_lock(&mut_msg_tx);
			val = message_queue_del_stamp(&msg_q_tx_req_h, msg->stamp);
			if(val > 0){//if req still in the queue 
				if(!memcmp(msg->data, sys.id, MSG_LEN_ID_GW)){//if head equals to the gw id
					sys.lic_status = LIC_VALID;
					memcpy(sys.auth_code, msg->data, SYS_LEN_AUTHCODE); 
					//After lic validated, send back an null message to server
					//printf("get auth ack, send null for test \n");
					msg_tx = message_create_null(sys.id, sys.tx_msg_stamp++); 
					msg_q_tx = message_queue_put(msg_q_tx, msg_tx);
					message_destroy(msg_tx);
				}
				//any auth ack msg not start with GW_ID will invalidate system
				else {
					sys.lic_status = LIC_INVALID;
				}
				pthread_mutex_unlock(&mut_msg_tx);
				result = 0;
			}
			else {//if auth is not acked, do nothing
				pthread_mutex_unlock(&mut_msg_tx);
				result = -1;
			}
			break;

		case DATA_ACK_STAMP:
			printf("received data ack stamp\n");
			vall = 0;
			memcpy(&(vall), msg->data, 4);
			sys.u_stamp = vall;
			for(idx = 0; idx < ZNET_SIZE; idx++) {
				if(!znode_isempty(&(sys.znode_list[idx]))){
					//this will only excute after stamp is synchronized with the server
					sys.znode_list[idx].u_stamp = sys.u_stamp;
				}
			}
			result = 0;
			break;
		case DATA_SET_INSTALL:
			len_descrip = msg->data_len - 2;
			printf("received set install, id = %s, dev type = %d\n", msg->dev_id, msg->dev_type);
			sys_edit_dev_install(&sys, msg->dev_id, msg->dev_type, len_descrip, msg->data+2);

			//send install data to znet
			msg_tx = message_create_install(msg->gateway_id, msg->dev_id, msg->dev_type);
			pthread_mutex_lock(&mut_msg_tx);
			msg_q_tx = message_queue_put(msg_q_tx, msg_tx);
			pthread_mutex_unlock(&mut_msg_tx);
			message_destroy(msg_tx);

			result = 0;
			break;
		case DATA_GET_INSTALL:
			//send install data to server
			printf("received get install\n");
			for (m = 0; m < ZNET_SIZE; m ++) {
				if (!znode_isempty( &(sys.znode_list[m])) ) {
					msg_tx = message_create_install_info(sys.id, sys.znode_install_list[m].id, sys.znode_install_list[m].type, sys.znode_install_list[m].len_descrip, sys.znode_install_list[m].descrip);
					pthread_mutex_lock(&mut_msg_tx);
					msg_q_tx = message_queue_put(msg_q_tx, msg_tx);
					pthread_mutex_unlock(&mut_msg_tx);
					message_destroy(msg_tx);
				}
			}
			result = 0;
			break;
		case DATA_DEL_INSTALL:
			idx = sys_del_dev_install(&sys, msg->dev_id);
			if (idx > 0) {
				msg_tx = message_create_del_install(msg->gateway_id, msg->dev_id);
				pthread_mutex_lock(&mut_msg_tx);
				msg_q_tx = message_queue_put(msg_q_tx, msg_tx);
				pthread_mutex_unlock(&mut_msg_tx);
				message_destroy(msg_tx);
			}
			
			result = 0;
			break;

		case DATA_FINISH_INSTALL:
			sys_update_dev_install(&sys, FILE_INSTALL);

			result = 0;
			break;
		default:
			result = 0;
			break;
	}

	return result;
}

int handle_local_message(message *msg, localuser *usr){
	message *msg_tx;
	localbundle bundle;
	bundle.usr = usr;
	int retval = 0;
	int m;
	char* tx_result;
	int idx;

	int len_descrip;

	if( message_isvalid(msg) != 1) {
		retval = -1;
	}
	else {
		printf("received msg from localuser\n");
		switch (msg->data_type){
			case DATA_CTRL:
				printf("ctrl from localuser, dev type = %d, len = %d \n", msg->dev_type, msg->data_len);
				gettimeofday( &(usr->time_lastactive), NULL );
				if (msg->dev_type == DEV_GW_FAN){
					/*
					 * handle controlling of gw fan
					 */
					printf("received gw fan ctrl = %s\n", msg->data);
					if (*(msg->data) == CTRL_ON){
						fan_control(1);
						sys.fan_status = STAT_ON;
					}
					else if (*(msg->data) == CTRL_OFF){
						fan_control(0);
						sys.fan_status = STAT_OFF;
					}
					else {
						//do nothing
					}
				}
				else {
					/*
					 * send control directly to znet
					 */
					msg_tx = message_create_ctrl(msg->data_len, msg->data, msg->gateway_id, msg->dev_id, msg->dev_type);
					pthread_mutex_lock(&mut_msg_tx);
					msg_q_tx = message_queue_put(msg_q_tx, msg_tx);
					pthread_mutex_unlock(&mut_msg_tx);
					message_destroy(msg_tx);
				}
				retval = 0;
				break;

			case DATA_REQ_STAT:
				printf("stat request from localuser\n");
				gettimeofday( &(usr->time_lastactive), NULL );
				//if req device is null, set back the whole znet
				if (msg->dev_type == DEV_GW_FAN){
					/*
					 * sending back fan status
					 */
					printf("received status req for GW fan, fan status = %s \n", &(sys.fan_status));
					msg_tx = message_create_stat(1, &(sys.fan_status), sys.id, NULL_DEV, DEV_GW_FAN); 

					bundle.msg = msg_tx;
					pthread_create( &(usr->thrd_tx), NULL, run_localuser_tx, &bundle);
					//the thrd_tx should return a new 
					pthread_join(usr->thrd_tx, NULL);
					//don't forget to delete the message
					message_destroy(msg_tx);
				}
				else if ( !memcmp(msg->dev_id, NULL_DEV, 8) ) {
					for (m = 0; m < ZNET_SIZE; m ++) {
						if( !znode_isempty( &(sys.znode_list[m]) ) ){
							msg_tx = message_create_stat(sys.znode_list[m].status_len, sys.znode_list[m].status, sys.id, sys.znode_list[m].id, sys.znode_list[m].type);
							bundle.msg = msg_tx;
							pthread_create( &(usr->thrd_tx), NULL, run_localuser_tx, &bundle);
							//the thrd_tx should return a new 
							pthread_join(usr->thrd_tx, NULL);
							//don't forget to delete the message
							message_destroy(msg_tx);
						}
					}
				}
				else {
					idx = sys_get_znode_idx(&sys, msg->dev_id); 
					if (idx >= 0){
						msg_tx = message_create_stat(sys.znode_list[idx].status_len, sys.znode_list[idx].status, sys.id, sys.znode_list[idx].id, sys.znode_list[idx].type);
						bundle.msg = msg_tx;
						pthread_create( &(usr->thrd_tx), NULL, run_localuser_tx, &bundle);
						//the thrd_tx should return a new 
						pthread_join(usr->thrd_tx, NULL);
						//don't forget to delete the message
						message_destroy(msg_tx);
					}
				}
				retval = 0;
				break;
			case DATA_SET_INSTALL:
				printf("set install info");
				gettimeofday( &(usr->time_lastactive), NULL );
				len_descrip = msg->data_len - 2;
				sys_edit_dev_install(&sys, msg->dev_id, msg->dev_type, len_descrip, msg->data+2);

				//send install data to znet
				msg_tx = message_create_install(msg->gateway_id, msg->dev_id, msg->dev_type);
				pthread_mutex_lock(&mut_msg_tx);
				msg_q_tx = message_queue_put(msg_q_tx, msg_tx);
				pthread_mutex_unlock(&mut_msg_tx);
				message_destroy(msg_tx);

				retval = 0;
				break;

			case DATA_GET_INSTALL:
				printf("retrive install info");
				//send install data to server
				gettimeofday( &(usr->time_lastactive), NULL );
				for (m = 0; m < ZNET_SIZE; m ++) {
					if (!znode_isempty( &(sys.znode_list[m])) ) {
						msg_tx = message_create_install_info(sys.id, sys.znode_install_list[m].id, sys.znode_install_list[m].type, sys.znode_install_list[m].len_descrip, sys.znode_install_list[m].descrip);
						bundle.msg = msg_tx;
						pthread_create( &(usr->thrd_tx), NULL, run_localuser_tx, &bundle);
						//the thrd_tx should return a new 
						pthread_join(usr->thrd_tx, NULL);
						//don't forget to delete the message
						message_destroy(msg_tx);
					}
				}
				retval = 0;
				break;

			case DATA_DEL_INSTALL:
				printf("deleted install\n");
				gettimeofday( &(usr->time_lastactive), NULL );
				idx = sys_del_dev_install(&sys, msg->dev_id);
				if (idx > 0) {
					msg_tx = message_create_del_install(msg->gateway_id, msg->dev_id);
					pthread_mutex_lock(&mut_msg_tx);
					msg_q_tx = message_queue_put(msg_q_tx, msg_tx);
					pthread_mutex_unlock(&mut_msg_tx);
					message_destroy(msg_tx);
				}
				retval = 0;
				break;

			case DATA_FINISH_INSTALL:
				printf("finished install, save to file\n");
				gettimeofday( &(usr->time_lastactive), NULL );
				sys_update_dev_install(&sys, FILE_INSTALL);
				retval = 0;
				break;
			case DATA_PULSE:
				gettimeofday( &(usr->time_lastactive), NULL );
				retval = 0;
				break;
			default:
				retval = -1;
				break;
		}
	}
	return retval;
}


