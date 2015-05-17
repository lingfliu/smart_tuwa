#include "twrt.h"

int main(int argn, char* argv[]){
	message *msg_auth;
	message *msg_stamp;

	//1. config the sys_t and the io
	/////////////////////////////////////
	get_config(&cfg);
	serial_config(cfg.serial_name, cfg.serial_type, cfg.serial_baudrate, &srl);
	inet_client_config(cfg.server_ip, cfg.server_port, cfg.server_proc, &client);

	//2. initialize buffers, queue, and sys
	/////////////////////////////////////
	buffer_ring_byte_create(&buff_serial, BUFF_RING_LEN);
	buffer_ring_byte_create(&buff_client, BUFF_RING_LEN);

	msg_q_rx = message_queue_create();
	message_queue_init(msg_q_rx); //save the head of the queue
	msg_q_rx_h = msg_q_rx;
	msg_q_tx = message_queue_create();
	message_queue_init(msg_q_tx);
	msg_q_tx_h = msg_q_tx;
	msg_q_tx_req = message_queue_create();
	message_queue_init(msg_q_tx_req);
	msg_q_tx_req_h = msg_q_tx_req;

	sys_init(&sys);

	//*************************************************
	//test code for config
	//printf("SERIAL_NAME=%s\n",cfg.serial_name);
	//printf("SERIAL_TYPE=%d\n",cfg.serial_type);
	//printf("SERIAL_BAUDRATE=%s\n",cfg.serial_baudrate);
	//printf("SERVER_IP=%s\n",cfg.server_ip);
	//printf("SERVER_PORT=%d\n",cfg.server_port);
	//printf("SERVER_PROC=%d\n",cfg.server_proc);
	//printf("HOST_PORT=%d\n",cfg.host_port);
	//printf("HOST_PROC=%d\n",cfg.host_proc);
	//printf("ID_SYS=%s\n",sys.id);
	//printf("LIC=%s\n",sys.lic);
	//*************************************************

	//3. connect server, open serial, initialize threads
	/////////////////////////////////////

	//mutex for the main thread
	pthread_mutex_init(&mut_sys, NULL);

	//connect the server
	while(inet_client_connect(&client) == -1){
		if(errno == EINPROGRESS){ //connection is in progress 
			//printf("Connecting\n");
			inet_timeout.tv_sec = 2; //set timeout as in 2 seconds
			inet_timeout.tv_usec = 0;
			FD_ZERO(&inet_fds);
			FD_SET(client.fd, &inet_fds);
			retval = select(client.fd+1, NULL, &inet_fds, NULL, &inet_timeout);
			if(retval == -1 || retval == 0){ //error or timeout
				//printf("Connection timeout\n");
				inet_client_close(&client);
				sys.server_status = SERVER_DISCONNECT;
				continue;
			}else{
				//printf("Connected\n");
				sys.server_status = SERVER_CONNECT;
				break;
			}
		}else{ //retry connecting to the server
			//printf("Connection error errno=%d\n",errno); 
			sys.server_status = SERVER_DISCONNECT;
			inet_client_close(&client);
			sleep(2); //sleep 1 second and try again
			continue;
		}
	}


	//open serial port
	while(serial_open(&srl) < 0 ){
		//perror("serial open failed\n");
		sleep(1);
	}


	//initialize sys_msg thread, mut, and cond
	pthread_mutex_init(&mut_msg_rx, NULL);
	pthread_mutex_init(&mut_msg_tx, NULL);

	if(pthread_create(&thrd_sys_msg_rx, NULL, run_sys_msg_rx, NULL) <0){
		perror("thrd_sys_msg_rx create");
		return -1;
	}

	if(pthread_create(&thrd_sys_msg_tx, NULL, run_sys_msg_tx, NULL) < 0){
		perror("thrd_sys_msg_tx create");
		return -1;
	}

	//initialize inet threads, mut, and cond
	pthread_mutex_init(&mut_client, NULL);
	pthread_cond_init(&cond_client, NULL);

	if(pthread_create(&thrd_client_rx, NULL, run_client_rx, NULL) < 0){
		perror("thrd_client_rx create");
		return -1;
	}

	if(pthread_create(&thrd_trans_client, NULL, run_trans_client, NULL) < 0){
		perror("thrd_trans_client create");
		return -1;
	}

	//initialize serial threads, mut, and cond
	pthread_mutex_init(&mut_serial,NULL);
	pthread_cond_init(&cond_serial, NULL);

	if(pthread_create(&thrd_serial_rx, NULL, run_serial_rx, NULL) < 0){
		perror("thrd_serial_rx create");
		return -1;
	}

	if(pthread_create(&thrd_trans_serial, NULL, run_trans_serial, NULL) < 0){
		perror("thrd_trans_serial create");
		return -1;
	}


	//initialize system periodic task
	gettimeofday(&(sys.timer_pulse), NULL);
	gettimeofday(&(sys.timer_reset), NULL);
	gettimeofday(&(sys.timer_sync), NULL);
	if(pthread_create(&thrd_sys_ptask, NULL, run_sys_ptask, NULL) < 0){
		perror("thrd_sys_ptask create");
		return -1;
	}

	msg_auth = message_create_req_auth_gw(SYS_LEN_LIC, sys.lic, sys.id, sys.tx_msg_stamp++); //create auth gw message
	msg_stamp = message_create_req_stamp(sys.id, sys.tx_msg_stamp++); //create auth gw message

	while(1){
		sleep(2);
		if(sys.lic_status == LIC_UNKNOWN){
			pthread_mutex_lock(&mut_msg_tx);
			if(message_queue_find_stamp(msg_q_tx_req_h, msg_auth->stamp) == 0){ //if previous auth req is not responded and is flushed
				//printf("Sending auth\n");
				msg_auth->stamp = sys.tx_msg_stamp++;
				msg_q_tx = message_queue_put(msg_q_tx, msg_auth);
			}else{
				//printf("Waiting for auth ack\n");
			}
			pthread_mutex_unlock(&mut_msg_tx);
		}

		if(sys.lic_status == LIC_INVALID){
			//printf("Lic not authed.\n");
			return -1;
		}

		//this will only run once 
		if(sys.lic_status == LIC_VALID && sys.u_stamp < 0){
			pthread_mutex_lock(&mut_msg_tx);
			if(message_queue_find_stamp(msg_q_tx_req, msg_stamp->stamp) == 0){
				msg_stamp->stamp = sys.tx_msg_stamp++;
				msg_q_tx = message_queue_put(msg_q_tx, msg_stamp);
			}
			pthread_mutex_unlock(&mut_msg_tx);
		}
	}
}

void *run_serial_rx(){
	pthread_detach(pthread_self());
	int len;
	while(1){
		usleep(5000);
		if(sys.lic_status == LIC_VALID){ //if server is disconnected, serial will keep working as long as the license is valid
			pthread_mutex_lock(&mut_serial);
			len = read(srl.fd, read_serial, SERIAL_BUFF_LEN);//non-blocking reading, return immediately
			if(len>0){
				//printf("incoming bytes\n");
				buffer_ring_byte_put(&buff_serial, read_serial, len);
				pthread_cond_signal(&cond_serial);
				pthread_mutex_unlock(&mut_serial);
			}else{
				if(errno == EAGAIN || errno == EINTR){ //reading in progress
					//printf("no byte\n"); 
					pthread_mutex_unlock(&mut_serial);
					continue;
				}else{ //io error 
					//printf("serial io error\n");
					serial_close(&srl);
					while(serial_open(&srl) < 0)
						usleep(5000); //wait 5 ms and try open serial again
					pthread_mutex_unlock(&mut_serial);
				}
			}
		}
	}
}

void *run_client_rx(){
	pthread_detach(pthread_self());
	int len;
	while(1){
		usleep(5000);
		if(sys.server_status == SERVER_CONNECT){
			pthread_mutex_lock(&mut_client);
			len = recv(client.fd, read_client, INET_BUFF_LEN, 0);//non-blocking reading, return immediately
			if(len>0){
				buffer_ring_byte_put(&buff_client, read_client, len);
				pthread_cond_signal(&cond_client);
			}else if(len == 0){//if disconnected, reconnect
				on_inet_client_disconnect();
			}else if(errno == EAGAIN || errno == EINTR ){
				//continue
			}
			pthread_mutex_unlock(&mut_client);
		}
	}
}

void *run_trans_serial(){
	pthread_detach(pthread_self());
	message *msg = message_create();
	int m;
	while(1){
		usleep(1000);
		pthread_mutex_lock(&mut_serial);
		pthread_cond_wait(&cond_serial, &mut_serial);
		//translate bytes into message
		for(m = 0; m < 35; m++)
		while(bytes2message(&buff_serial, msg)>0){
			pthread_mutex_lock(&mut_msg_rx);
			msg_q_rx = message_queue_put(msg_q_rx, msg);
			message_flush(msg);
			pthread_mutex_unlock(&mut_msg_rx);
			usleep(1000);
		}
		pthread_mutex_unlock(&mut_serial);
	}
}

void *run_trans_client(){
	pthread_detach(pthread_self());
	message *msg = message_create();
	while(1){
		usleep(5000);
		pthread_mutex_lock(&mut_client);
		pthread_cond_wait(&cond_client, &mut_client);
		//translate bytes into message
		while(bytes2message(&buff_client, msg)>0){
			pthread_mutex_lock(&mut_msg_rx);
			msg_q_rx = message_queue_put(msg_q_rx, msg);
			message_flush(msg);
			pthread_mutex_unlock(&mut_msg_rx);
			usleep(1000);
		}
		pthread_mutex_unlock(&mut_client);
	}
}

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
		}else{
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
					if(sys.lic_status != LIC_VALID){//if not authed, do not send
						break;
					}else{
						if(pthread_create(&thrd_serial_tx, NULL, run_serial_tx, (void*) msg) < 0){
							perror("thrd_serial_tx create");
							break;
						}
						pthread_join(thrd_serial_tx, NULL);
						break;
					}
				case MSG_TO_SERVER:
					if(sys.lic_status != LIC_VALID){//if not authed, send only auth msg
						if(msg->data_type == DATA_REQ_AUTH_GW){
							if(pthread_create(&thrd_client_tx, NULL, run_client_tx, (void*) msg) < 0){
								perror("thrd_client_tx create");
								break;
							}
							pthread_join(thrd_client_tx, NULL);
						}
						break;
					}else{
						if(pthread_create(&thrd_client_tx, NULL, run_client_tx, (void*) msg) < 0){
							perror("thrd_client_tx create");
							break;
						}
						pthread_join(thrd_client_tx, NULL);
						break;
					}
				default://if unknown, flush the message from the queue
					break;
			}
			message_flush(msg);
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

	while(pos < len){
	   ret = write(srl.fd, bytes+pos, len);
	   if(ret == len-pos){
		   free(bytes);
		   pthread_exit(0);
	   }

	   if(ret == -1){
		   if(errno == EAGAIN || errno == EINTR){
			   usleep(5000);
			   continue;
		   }else{
			   free(bytes); //don't forget to free the mem
			   serial_close(&srl);
			   while(serial_open(&srl) < 0)
				   usleep(5000); //wait 5 ms and try open serial again
			   pthread_exit(0);
		   }
	   }else{
			pos += ret;
	   }
	}
	free(bytes); //don't forget to free the mem
	pthread_exit(0);
}

void *run_client_tx(void *arg){
	message *msg = (message*)arg;
	int len = MSG_LEN_FIXED+msg->data_len;
	char *bytes = calloc(len,sizeof(char)); 
	message2bytes(msg, bytes);
	int ret; 
	int pos = 0;

	if(len == 0)
		pthread_exit(0);
		//return;

	while(pos < len && sys.server_status == SERVER_CONNECT){
		ret = send(client.fd, bytes+pos, len - pos, 0);
		if(ret == len - pos){
			free(bytes); //don't forget to free the mem
			pthread_exit(0);
		}

		if(ret == -1){ //send failed
			if(errno == EAGAIN || errno == EINTR){ //buff is full or interrupted
				usleep(1000);
				continue;
			}
			if(errno == ECONNRESET){ //connection broke
				free(bytes);
				on_inet_client_disconnect();
				pthread_exit(0);
			}
			free(bytes); //don't forget to free the mem
			pthread_exit(0);
		}else{ //send partial data
			pos += ret;
		}
	}
	free(bytes); //don't forget to free the mem
	pthread_exit(0);
}

void* run_sys_ptask(){
	pthread_detach(pthread_self());

    struct timeval timer;
    message* msg = message_create();
    int m;

	while(1){	
		//periodic tasks
		sleep(1);
		gettimeofday(&timer, NULL);

		//req msg cleaning
		pthread_mutex_lock(&mut_msg_tx);

		//if no req in the queue
		if(message_queue_getlen(msg_q_tx_req_h) == 0){
		}else{
			while(timediff(msg_q_tx_req_h->time, timer)>=TIMER_REQ){
				msg_q_tx_req_h = message_queue_get(msg_q_tx_req_h, msg); //remove the un-responed msg
				message_flush(msg);
				if(message_queue_getlen(msg_q_tx_req_h) == 0) //if queue is empty
					break;
			}
		}
		pthread_mutex_unlock(&mut_msg_tx);

		//sync the gw and znet
		if(timediff(sys.timer_sync, timer)>TIMER_SYNC){
			for(m = 0; m<PROC_ZNODE_MAX; m++){//synchronize the znodes
				if(!znode_isempty(&(sys.znode_list[m]))){
					message_destroy(msg); //destroy old message before cerating one
					msg = message_create_sync(sys.znode_list[m].status_len, sys.znode_list[m].status, sys.znode_list[m].u_stamp, sys.id, sys.znode_list[m].id, sys.znode_list[m].type, 0);
					pthread_mutex_lock(&mut_msg_tx);
					msg_q_tx = message_queue_put(msg_q_tx, msg);//send the hb to the server
					pthread_mutex_unlock(&mut_msg_tx);
				}
			}

			//synchronize the root
			message_destroy(msg);
			msg = message_create_sync(SYS_LEN_STATUS, sys.status, sys.u_stamp, sys.id, NULL_DEV, 0, 0);
			pthread_mutex_lock(&mut_msg_tx);
			msg_q_tx = message_queue_put(msg_q_tx, msg);//send the hb to the server
			pthread_mutex_unlock(&mut_msg_tx);

			//reset the timer
			gettimeofday(&(sys.timer_sync), NULL);
		}
		

		//tcp pulse
		
		if(timediff(sys.timer_pulse, timer)>TIMER_PULSE){
			message_destroy(msg);
			msg = message_create_pulse(sys.id, 0);
			pthread_mutex_lock(&mut_msg_tx);
			msg_q_tx = message_queue_put(msg_q_tx, msg);//send the hb to the server
			gettimeofday(&(sys.timer_pulse), NULL); //update the timer
			pthread_mutex_unlock(&mut_msg_tx);

			//reset the timer
			gettimeofday(&(sys.timer_pulse), NULL);
		}
		
		/*
		//reset
		if(timediff_hour(timer, sys->timer_reset)>TIMER_RESET){
		msg = message_create_pulse(sys->id_gw, 0);
		pthread_mutex_lock(&mut_msg_tx);
		msg_q_tx = message_queue_put(msg_q_tx, msg);//send the hb to the server
		sys->timer_pulse = gettimeofday(); //update the timer
		pthread_mutex_unlock(&mut_msg_tx);
		}*/
	}
}

//message handle function
int handle_msg_rx(message *msg){
	message *msg_tx;
	int idx;
	int result = 0;
	int val;
	if(!message_isvalid(msg))
		return;

	switch(msg->data_type){
		case DATA_STAT: 
			//update stat
			idx = sys_znode_update(&sys, msg);
			//if msg is a valid stat msg, sync to server
			if(idx>=0){
				msg_tx = message_create_sync(sys.znode_list[idx].status_len, sys.znode_list[idx].status, sys.znode_list[idx].u_stamp, sys.id, sys.znode_list[idx].id, sys.znode_list[idx].type, 0);
				pthread_mutex_lock(&mut_msg_tx);
				msg_q_tx = message_queue_put(msg_q_tx, msg_tx);
				pthread_mutex_unlock(&mut_msg_tx);
				message_destroy(msg_tx);
				result = 0;
			}
			break;

		case DATA_CTRL:
			//send ctrl to znet
			msg_tx = message_create_ctrl(msg->data_len, msg->data, msg->gateway_id, msg->dev_id, sys.tx_msg_stamp++);
			pthread_mutex_lock(&mut_msg_tx);
			msg_q_tx = message_queue_put(msg_q_tx, msg_tx);
			pthread_mutex_unlock(&mut_msg_tx);
			message_destroy(msg_tx);
			result = 0;
			break;

		case DATA_REQ_SYNC:
			msg_tx = sys_sync(&sys, msg);
			if(msg_tx == NULL){ //if server status is newer than local
				result = 0;
				break;
			}else{
				pthread_mutex_lock(&mut_msg_tx);
				msg_q_tx = message_queue_put(msg_q_tx, msg_tx);
				pthread_mutex_unlock(&mut_msg_tx);
				message_destroy(msg_tx);
				result = 0;
				break;
			}

		case DATA_ACK_AUTH_GW:
			pthread_mutex_lock(&mut_msg_tx);
			val = message_queue_del_stamp(&msg_q_tx_req_h, msg->stamp);
			if(val > 0){//if req still in the queue 
				if(!memcmp(msg->data, sys.id, MSG_LEN_ID_GW)){//if head not equals to the gw id
					sys.lic_status = LIC_VALID;
					memcpy(sys.cookie, msg->data, SYS_LEN_COOKIE); 
					//After lic validated, send back an null message to server
					msg_tx = message_create_null(sys.id, sys.tx_msg_stamp++); 
					msg_q_tx = message_queue_put(msg_q_tx, msg_tx);
					message_destroy(msg_tx);
				}else{
					sys.lic_status = LIC_INVALID;
				}
				pthread_mutex_unlock(&mut_msg_tx);
				result = 0;
			}else{//if auth is not acked, do nothing
				pthread_mutex_unlock(&mut_msg_tx);
				result = -1;
			}
			break;

		case DATA_ACK_STAMP:
			memcpy(&(sys.u_stamp), msg->data, 4);
			for (idx = 0; idx < PROC_ZNODE_MAX; idx++){
				if(!znode_isempty(&(sys.znode_list[idx]))){
					sys.znode_list[idx].u_stamp = sys.u_stamp;
				}
			}
			result = 0;
			break;

		case DATA_ACK_AUTH_DEV:
			pthread_mutex_lock(&mut_msg_tx);
			val = message_queue_del_stamp(&msg_q_tx_req_h, msg->stamp);
			idx = sys_get_znode_idx(&sys, msg->data);
			if(val > 0){//if stamp still in the queue 
				if(memcmp(msg->data, sys.znode_list[idx].id, MSG_LEN_ID_DEV)){//if head not equals to the gw id
					//to-do
				}else{
					//to-do
				}
				pthread_mutex_unlock(&mut_msg_tx);
				result = 0;
			}else{//otherwise do nothing
				pthread_mutex_unlock(&mut_msg_tx);
				result = -1;
			}
			break;

		default:
			result = 0;
			break;
	}

	return result;
}

void on_inet_client_disconnect(){
    //when disconnected, clear the cookie and set lic status as unknown
	sys.server_status = SERVER_DISCONNECT;
    sys.lic_status = LIC_UNKNOWN;
    memset(sys.cookie, 0, SYS_LEN_COOKIE);
    
	inet_client_close(&client); //close the connection first
	//connect the server
	while(inet_client_connect(&client) == -1){
		if(errno == EINPROGRESS){ //connection is in progress 
			inet_timeout.tv_sec = 2; //set timeout as in 2 seconds
			inet_timeout.tv_usec = 0;
			FD_ZERO(&inet_fds);
			FD_SET(client.fd, &inet_fds);
			retval = select(client.fd+1, NULL, &inet_fds, NULL, &inet_timeout);
			if(retval == -1 || retval == 0){ //error or timeout
				inet_client_close(&client);
				sys.server_status = SERVER_DISCONNECT;
				continue;
			}else{
				sys.server_status = SERVER_CONNECT;
			    return;	
			}
		}else{ //retry connecting to the server
			sys.server_status = SERVER_DISCONNECT;
			inet_client_close(&client);
			sleep(2); //sleep 1 second and try again
			continue;
		}
	}
	sys.server_status = SERVER_CONNECT;
}

long timediff(struct timeval time_before, struct timeval time_after){
    long val;
	long sec = (time_after.tv_sec - time_before.tv_sec)*1000; 
	long usec = (time_after.tv_usec - time_before.tv_usec)/1000;
	val = sec+usec;
    return val;
}

long timediff_hour(struct timeval time_before, struct timeval time_after){
    long val;
	val = (time_after.tv_sec - time_before.tv_sec)/3600; 
    return val;
}
