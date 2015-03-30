#include "twrt.h"

int main(int argn, char* argv[]){
    message *msg;
    char* bytes;
    int len;
    int len_rx;
    int timer;

    //1. config the system and the io
    /////////////////////////////////////
    get_config(&cfg);
    serial_config(cfg->serial_name, cfg->serial_type, cfg->serial_baudrate, &srl);
    inet_client_config(cfg->server_ip, cfg->server_port, cfg->server_proc, &inet_client);
    
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
    msg_q_tx_req_h = msg_q_tx_req_h;

    sys_init(&sys);

    //3. initialize inet threads and sys_msg threads
    /////////////////////////////////////
    //connect the server
    while(inet_client_connect(&client)<0){
	sleep(1);
    }//retry if not connected

    msg = message_create_req_auth_gw(SYS_LEN_LIC, sys->lic, sys->id, 0);
    message_queue_put(msg_q_tx, msg);

    pthread_mutex_init(&mut_client);
    pthread_cond_init(&cond_client);
    pthread_mutex_init(&mut_msg_rx);
    pthread_cond_init(&cond_msg_rx);
    pthread_mutex_init(&mut_msg_tx);
    pthread_cond_init(&cond_msg_tx);

    pthread_create(thrd_client_rx, run_client, NULL);
    pthread_create(thrd_trans_client, run_trans_client, NULL);
    pthread_create(thrd_sys_msg_rx, run_sys_msg_rx, NULL);
    pthread_create(thrd_sys_msg_tx, run_sys_msg_tx, NULL);

    while(sys->lic_status == UNKNOWN){//wait until the gw get authed
	usleep(50000); //sleep 50ms 
	pthread_mutex_lock(&mut_msg_tx);
	if(message_queue_getlen(msg_q_tx_req)==0){ //if previous auth req is not responded
	    msg_q_tx = message_queue_put(msg_q_tx, msg); //send another one
	}
	pthread_mutex_unlock(&mut_msg_tx);
    }
    //check the lic validity
    if(sys->lic_status == INVALID){
	perror("lic invalid");
	exit -1;
    }

    //flush all auth msg
    msg_q_rx = message_queue_flush(msg_q_rx); 
    msg_q_tx = message_queue_flush(msg_q_tx); 
    msg_q_tx_req = message_queue_flush(msg_q_tx_req); 


    //4. if lic is valid, retrieve stamp from the web
    /////////////////////////////////////
    messag_destroy(msg);
    msg = message_create_req_stamp(sys->id_gw);
    while(sys->stamp <= 0){//wait until the gw get stamp
	if(message_queue_getlen(msg_q_tx_req)==0){ //if previous auth req is not responded
	    message_queue_put(msg_q_tx, msg); //send another one
	}
	usleep(50000); //sleep 50ms 
    }

    //flush all req stamp msg
    msg_q_rx = message_queue_flush(msg_q_rx); 
    msg_q_tx = message_queue_flush(msg_q_tx); 
    msg_q_tx_req = message_queue_flush(msg_q_tx_req); 

    //5. start serial threads
    /////////////////////////////////////
    //open serial port
    serial_open(&serial);


    pthread_mutex_init(&mut_serial);
    pthread_cond_init(&cond_serial);

    pthread_create(thrd_serial_rx, run_serial_rx, NULL);
    pthread_create(thrd_trans_serial, run_trans_serial, NULL);
    pthread_create(thrd_sys_ptask, run_sys_ptask, NULL);

    for(;;);
}

void *run_serial_rx(){
    int len;
    while(1){
	usleep(5000);
	if(sys->server_status == SERVER_CONNECT && sys->lic_status == LIC_VALID){
	    pthread_mutex_lock(&mut_serial);
	    len = read(serial->fd, read_serial, SERIAL_BUFF_LEN);//non-blocking reading, return immediately
	    if(len>0){
		buffer_ring_byte_put(buffer_serial, read_serial, len);
		pthread_cond_signal(&cond_serial);
	    }else if(len == 0){
		//continue
	    }else{
		//if i/o errror
		serial_close(serial);
		while(serial_open(serial) < 0)
		    usleep(5000); //wait 5 ms and try open serial again
	    }
	    pthread_mutex_unlock(&mut_serial);
	}
    }
}

void *run_client_rx(){
    int len;
    while(1){
	usleep(5000);
	if(sys->server_status == SERVER_CONNECT){
	    pthread_mutex_lock(&mut_client);
	    len = recv(client->fd, read_client, INET_BUFF_LEN);//non-blocking reading, return immediately
	    if(len>0){
		buffer_ring_byte_put(buff_client, read_client, len);
		pthread_cond_signal(&cond_client);
	    }else if(len == 0){//if disconnected, reconnect
		sys->server_status = SERVER_DISCONNECT;//stop the thread 
		on_inet_client_disconnect();
	    }else if(errno == EAGAIN | errno == EINTR ){
		//continue
	    }else{ //network disconnect
		sys->server_status = SERVER_DISCONNECT;
		on_inet_client_disconnect();
	    }
	    pthread_mutex_unlock(&mut_client);
	}
    }
}

void *run_trans_serial(){
    message *msg = message_create();
    while(1){
	usleep(1000);
	pthread_mutex_lock(&mut_serial);
	pthread_cond_wait(&cond_serial, &mut_serial);
	//translate bytes into message
	while(bytes2message(buff_serial, msg)>0){
	    usleep(1);
	    pthread_mutex_lock(&mut_msg_rx);
	    msg_q_rx = message_queue_put(msg_q_rx, msg);
	    message_destroy(msg);
	    pthread_mutex_unlock(&mut_msg_rx);
	}
	pthread_mutex_unlock(&mut_serial);
    }
}

void *run_trans_client(){
    message *msg = message_create();
    while(1){
	usleep(1000);
	pthread_mutex_lock(&mut_client);
	pthread_cond_wait(&cond_client, &mut_client);
	//translate bytes into message
	while(bytes2message(buff_client, msg)>0){
	    usleep(1);
	    pthread_mutex_lock(&mut_msg_rx);
	    msg_q_rx = message_queue_put(msg_q_rx, msg);
	    message_destroy(msg);
	    pthread_mutex_unlock(&mut_msg_rx);
	}
	pthread_mutex_unlock(&mut_client);
    }
}

void *run_sys_msg_rx(){
    message *msg = message_create();
    while(1){
	usleep(1000);
	pthread_mutex_lock(&mut_msg_rx);
	if(message_queue_getlen(msg_q_rx_h)>0){
	    mesg_q_rx_h = message_queue_get(msg_q_rx_h, msg); //read from the head
	    pthread_mutex_unlock(&mut_msg_rx);//unlock msg_q_rx first
	    handle_msg_rx(msg);
	    message_destroy(msg);
	}else{
	    pthread_mutex_unlock(&mut_msg_rx);//unlock msg_q_rx first
	    message_destroy(msg);
	}
    }
}

void *run_sys_msg_tx(){
    message *msg = message_create();

    while(1){
	usleep(1000);
	pthread_mutex_lock(&mut_msg_tx);
	if(message_queue_getlen(msg_q_tx_h)<=0){//if empty, do nothing
	    pthread_mutex_unlock(&mut_msg_tx);
	}else{
	    msg_q_tx_h = message_queue_get(msg_q_tx_h, msg);
	    if(message_isreq(msg)){ //put req msg into req queue, add timeval to the req
		msg_q_tx_req = message_queue_put(msg_q_tx_req, msg);
		msg_q_tx_req->time = gettimeofday();
	    }

	    switch(message_tx_dest(msg)){
		case MSG_TO_SERIAL: 
		    if(sys->lic_status != LIC_VALID){//if not authed, do not send
			break;
		    }else{
			pthread_create(thrd_serial_tx, run_serial_tx, msg);
			pthread_join(thrd_serial_tx);
			break;
		    }
		case MSG_TO_SERVER:
		    if(sys->lic_status != LIC_VALID){//if not authed, send only auth msg
			if(msg->data_type == DATA_REQ_AUTH_GW){
			pthread_create(thrd_client_tx, run_serial_tx, msg);
			pthread_join(thrd_client_tx);
			}
			break;
		    }else{
			pthread_create(thrd_client_tx, run_serial_tx, msg);
			pthread_join(thrd_client_tx);
			break;
		    }
		default://if unknown, flush the message from the queue
		    break;
	    }
	    message_destroy(msg);
	    pthread_mutex_unlock(&mut_msg_tx);
	}
    }
}

void *run_serial_tx(void *arg){
    message *msg = (message)arg;
    char *bytes;
    int len = message2bytes(msg, &bytes);
    if(send(srl->fd, bytes, len, 0)<=0){
	perror("failed to send to server");
	free(bytes); //don't forget to free the mem
	pthread_exit();
    }
    free(bytes); //don't forget to free the mem
    pthread_exit();
}

void *run_client_tx(void *arg){
    message *msg = (message)arg;
    char *bytes;
    int len = message2bytes(msg, &bytes);
    if(send(client->fd, bytes, len, 0)<=0){
	perror("failed to send to server");
	free(bytes); //don't forget to free the mem
	pthread_exit();
    }
    free(bytes); //don't forget to free the mem
    pthread_exit();
}

void* run_sys_ptask(){
    struct timeval timer;
    message* msg;
    int m;
    int num;

    while(1){	
	//periodic tasks
	usleep(5000);
	timer = gettimeofday();

	//req msg cleaning
	pthread_mutex_lock(&mut_msg_tx);
	while(timediff(msg_q_tx_req_h->time, timer)>=TIMER_REQ){
	    msg_q_tx_req_h = message_queue_get(msg_q_tx_req_h, msg); //remove the un-responed msg
	    message_destroy(msg);
	}
	pthread_mutex_unlock(&mut_msg_tx);

	//sync the gw and znet
	if(timediff(sys->timer_sync, timer)>TIMER_SYNC){
	   for(m = 0; m<PROC_ZNODE_MAX; m++){//synchronize the znodes
	       if(!znode_isempty(sys->znode_list[m])){
		   msg = message_create_sync(sys->znode_list[m]->status_len, sys->znode_list[m]->status, sys->znode_list[m]->u_stamp, sys->id, sys->znode_list[m]->id, 0);
		   pthread_mutex_lock(&mut_msg_tx);
		   msg_q_tx = message_queue_put(msg_q_tx, msg);//send the hb to the server
		   pthread_mutex_unlock(&mut_msg_tx);
		   message_destroy(msg);
	       }
	   }
	   //synchronize the root
	   msg = message_create_sync(SYS_LEN_STATUS, sys->status, sys->u_stamp, sys->id, sys->znode_list[m]->id, NULL_DEV, 0);
	   pthread_mutex_lock(&mut_msg_tx);
	   msg_q_tx = message_queue_put(msg_q_tx, msg);//send the hb to the server
	   pthread_mutex_unlock(&mut_msg_tx);
	   message_destroy(msg);
	   
	   //reset the timer
	   sys->sync = timer;
	}

	//tcp pulse
	if(timediff(timer, sys->timer_pulse)>TIMER_PULSE){
	    msg = message_create_pulse(sys->id_gw, 0);
	    pthread_mutex_lock(&mut_msg_tx);
	    msg_q_tx = message_queue_put(msg_q_tx, msg);//send the hb to the server
	    sys->timer_pulse = gettimeofday(); //update the timer
	    pthread_mutex_unlock(&mut_msg_tx);

	    //reset the timer
	    sys->pulse = timer;
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
    int result;
    int val;
    switch(msg->data_type){
	case DATA_STAT: 
	    //update stat
	    idx = sys_znode_update(sys, msg);
	    //if msg is a valid stat msg, sync to server
	    if(idx>=0){
		msg_tx = message_create_sync(sys->znode_list[idx]->stat_len, sys->znode_list[idx]->stat, sys->id, sys->znode_list[idx]->id, sys->tx_msg_stamp++);
		pthread_mutex_lock(&mut_msg_tx);
		msg_q_tx = message_queue_put(msg_q_tx, msg_tx);
		pthread_mutex_unlock(&mut_msg_tx);
		message_destroy(msg_tx);
		result = 0;
	    }
	    break;

	case DATA_CTRL:
	    //send ctrl to znet
	    msg_tx = message_create_ctrl(msg->data_len, msg->data, msg->msg->id_gw, msg->id_dev, sys->tx_msg_stamp++);
	    pthread_mutex_lock(&mut_msg_tx);
	    msg_q_tx = message_queue_put(msg_q_tx, msg_tx);
	    pthread_mutex_unlock(&mut_msg_tx);
	    message_destroy(msg_tx);
	    result = 0;
	    break;

	case DATA_REQ_SYNC:
	    msg_tx = sys_sync(sys, msg);
	    if(msg_tx == NULL){ //if server status is newer than local
		result = 0;
		break;
	    }else{
		pthread_mutex_lock(&mut_msg_tx);
		msg_q_tx = message_queue_put(msg_q_tx, msg_tx);
		pthread_mutex_unlock(&mut_msg_tx);
		result = 0;
		break;
	    }

	case DATA_ACK_AUTH_GW:
	    pthread_mutext_lock(&mut_msg_tx);
	    val = message_queue_del_stamp(msg_q_tx_req_h, msg->stamp);
	    if(val > 0){//if stamp still in the queue 
		if(memcmp(msg->data, sys->id, MSG_LEN_ID_GW)){//if head not equals to the gw id
		    sys->lic_status = LIC_VALID;
		    memcpy(sys->cookie, msg->data, MSG_LEN_COOKIE); 
		}else{
		    sys->lic_status = LIC_INVALID;
		}
		pthread_mutext_unlock(&mut_msg_tx);
		result = 0;
	    }else{//otherwise donothing
		pthread_mutext_unlock(&mut_msg_tx);
		result = -1;
	    }
	    break;

	case DATA_ACK_AUTH_DEV:
	    pthread_mutext_lock(&mut_msg_tx);
	    val = message_queue_del_stamp(msg_q_tx_req_h, msg->stamp);
	    idx = sys_get_znode_idx(sys, msg->data);
	    if(val > 0){//if stamp still in the queue 
		if(memcmp(msg->data, sys->znode_list[idx], MSG_LEN_ID_DEV)){//if head not equals to the gw id
		    //to-do
		}else{
		    //to-do
		}
		pthread_mutext_unlock(&mut_msg_tx);
		result = 0;
	    }else{//otherwise donothing
		pthread_mutext_unlock(&mut_msg_tx);
		result = -1;
	    }
	    break;

	case DATA_ACK_STAMP:
	    memcpy(sys->u_stamp, msg->data, 4);
	    for(idx = 0; idx < PROC_ZNODE_MAX; idx++)
		sys->znode_list[idx]->u_stamp = u_stamp;
	    result = 0;
	    break;

	default:
	    break;
    }
    return result;
}

void on_inet_client_disconnect(){
    int m;
    //when disconnected clear the cookie and set lic status as unknown
    sys->lic_status = LIC_UNKNOWN;
    memset(sys->cookie, 0, SYS_LEN_COOKIE);

    pthread_mutex_lock(&mut_msg_rx);
    pthread_mutex_lock(&mut_msg_tx);
    
    //flush the message queue
    msg_q_rx = message_queue_flush(msg_q_rx);
    msg_q_rx_h = msg_q_rx;
    msg_q_tx = message_queue_flush(msg_q_tx);
    msg_q_tx_h = msg_q_tx;
    msg_q_tx_req = message_queue_flush(msg_q_tx_req);
    msg_q_tx_req_h = msg_q_tx_req;

    while(inet_client_connect(&client) < 0)
	sleep(1); //sleep 1 second and try again
    sys->server_status = SERVER_CONNECT;

    pthread_mutex_unlock(&mut_msg_rx);
    pthread_mutex_unlock(&mut_msg_tx);
}

int timediff(struct timeval time_before, struct timeval time_after){
    int val;
    struct timeval tdiff;
    time_substract(&tdiff, &time_before, &time_after);
    val = (int) (tdiff.td_sec*1000+tdiff.td_usec/1000);
    return val;
}

int timediff_hour(struct timeval time_before, struct timeval time_after){
    int val;
    struct timeval tdiff;
    time_substract(&tdiff, &time_before, &time_after);
    val = (int) (tdiff.td_sec/3600);
    return val;
}
