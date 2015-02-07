#include "twrt.h"

int main(int argn, char* argv[]){
    //1. get config
    /////////////////////////////////////
    get_config(&cfg);
    serial_config(cfg->serial_name, cfg->serial_type, cfg->serial_baudrate, &serial);
    inet_client_config(cfg->server_ip, cfg->server_port, cfg->server_proc, &inet_client);
    
    //2. initialize buffers, paras, and sys
    /////////////////////////////////////
    buffer_byte_ring_create(buff_serial, BUFF_RING_LEN);
    buffer_byte_ring_create(buff_client, BUFF_RING_LEN);
    memset(read_serial, 0, sizeof(read_serial));
    memset(read_client, 0, sizeof(read_client));
    message_queue_init(msg_q_rx);
    message_queue_init(msg_q_tx);
    msg_stamp = 0;
    //time_net_hb = time();
    //time_sys_reset = time();

    serial_open(&serial);
    while(inet_client_connect(&client)!= 0)
	sleep(1); //sleep 1 second and try again

    //3. retrieve authorization
    /////////////////////////////////////
    if(sys_get_auth(sys, client.fd)!=0){
	perror("Device not authorized, please check your liscense file\n");
	return -1;
    }
    //5. retrieve stamps from the web
    /////////////////////////////////////
    sys_get_stamp(sys, client.fd);

    //6. initialize synchronization to the server and the local znet 
    /////////////////////////////////////
    sys_sync_init(sys, serial.fd, client.fd);

    //7. start threads
    
    pthread_mutex_init(&mut_serial);
    pthread_cond_init(&cond_serial);
    pthread_mutex_init(&mut_client);
    pthread_cond_init(&cond_client);
    pthread_mutex_init(&mut_msg_rx);
    pthread_cond_init(&cond_msg_rx);
    pthread_mutex_init(&mut_msg_tx);
    pthread_cond_init(&cond_msg_tx);

    pthread_create(thrd_serial_rx, run_serial_rx, NULL);
    pthread_create(thrd_client_rx, run_client, NULL);
    pthread_create(thrd_trans_serial, run_trans_serial, NULL);
    pthread_create(thrd_trans_client, run_trans_client, NULL);
    pthread_create(thrd_sys_msg_rx, run_sys_msg_rx, NULL);
    pthread_create(thrd_sys_msg_tx, run_sys_msg_tx, NULL);
    pthread_create(thrd_sys_ptask, run_sys_ptask, NULL);

    for(;;);
}

void *run_serial_rx(){
    int len;
    while(1){
	usleep(1000);
	pthread_mutex_lock(&mut_serial);
	len = read(serial->fd, read_serial, SERIAL_BUFF_LEN);//non-blocking reading, return immediately
	if(len>0){
	    put_buffer_byte_ring(buffer_serial, read_serial, len);
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

void *run_client_rx(){
    int len;
    while(1){
	usleep(1000);
	pthread_mutex_lock(&mut_client);
	len = read(client->fd, read_client, INET_BUFF_LEN);//non-blocking reading, return immediately
	if(len>0){
	    put_buffer_byte_ring(buff_client, read_client, len);
	    pthread_cond_signal(&cond_client);
	}else if(errno == EAGAIN | errno == EINTR ){
	    //continue
	}else if(len == 0){//if disconnected, reconnect
	    while(inet_client_connect(client)<0)
		sleep(1);// wait 1 s and try connect again
	}else{
	    while(inet_client_connect(client)<0)
		sleep(1);// wait 1 s and try connect again
	}
	pthread_mutex_unlock(&mut_client);
    }
}

void *run_trans_serial(){
    struct_message *msg;
    msg = message_create_empty();
    while(1){
	usleep(1000);
	pthread_mutex_lock(&mut_serial);
	pthread_cond_wait(&cond_serial, &mut_serial);
	//translate bytes into message
	while(bytes2msg(buff_serial, msg)>0){
	    usleep(1);
	    pthread_mutex_lock(&mut_msg_rx);
	    message_queue_put(msg_q_rx, msg);
	    message_del(msg);
	    pthread_mutex_unlock(&mut_msg_rx);
	}
	pthread_mutex_unlock(&mut_serial);
    }
}

void *run_trans_client(){
    struct_message *msg;
    while(1){
	usleep(1000);
	pthread_mutex_lock(&mut_client);
	pthread_cond_wait(&cond_client, &mut_client);
	//translate bytes into message
	while(bytes2msg(buff_client, msg)>0){
	    usleep(1);
	    pthread_mutex_lock(&mut_msg_rx);
	    message_queue_put(msg_q_rx, msg);
	    message_flush(msg);
	    pthread_mutex_unlock(&mut_msg_rx);
	}
	pthread_mutex_unlock(&mut_client);
    }
}

void *run_sys_msg_rx(){
    struct_message *msg;
    message_flush(msg);
    while(1){
	usleep(1000);
	pthread_mutex_lock(&mut_msg_rx);
	if(message_queue_isempty(msg_q_rx))
		continue;
	else
	    message_queue_get_head(msg_q_rx, msg);
	pthread_mutex_unlock(&mut_msg_rx);//unlock msg_q_rx first
	if(msg != NULL){
	    handle_msg(msg);
	    message_flush(msg);
	}
    }
}

void *run_sys_msg_tx(){
    struct_message *msg;
    struct_message_queue msg_q_item;
    while(1){
	usleep(1000);
	pthread_mutex_lock(&mut_msg_tx);
	if(message_queue_isempty(msg_q_tx))
	    continue;
	else{
	    msg_q_item = message_queue_head(msg_q_tx);
	    while(message_is_req(msg_q_item->msg))//if is req msg
	    if(mesg_q_item->msg->timer == NULL){//unsend req
		msg_q_item->msg->timer = time();
		break;//when finish sending the first unsend msg, break
	    }else{
		msg_q_item = msg_q_item->next;
		//if send before
		//go to the next message
	    }

	    //skipped all send req msg and locate the first unsend msg
	    //identify the write direction
	    switch(message_direction(msg_q_item->msg)){
		case MSG_DIR_SERIAL:
		    pthread_create(thrd_serial_tx, run_serial_tx, msg);
		    pthread_join(thrd_serial_tx);
		    break;
		case MSG_DIR_INET_CLIENT:
		    pthread_create(thrd_client_tx, run_serial_tx, msg);
		    pthread_join(thrd_client_tx);
		    break;
		default://if unknown, flush the message from the queue
		    if(msg_q_item == msg_q_tx)
			message_queue_flush(msg_q_tx);
		    else
			message_queue_flush(msg_q_item);
		    break;
	    }
	}
	pthread_mutex_unlock(&mut_msg_tx);
    }
}

void* run_sys_ptask(){
    time_t timer;
    time_t timer_net_hb = time();
    time_t timer_sys_reset = time();

    struct_message_queue* msg_q_item;
    struct_message* msg;

    while(1){	
	//periodic tasks
	msleep(100);
	timer = time();

	//msg_q_tx clean
	pthread_mutex_lock(&mut_msg_tx);
	while(msg_q_tx->next!=msg_q_tx){
	    message_queue_read(msg_q_tx, msg);
	    if(difftime(timer, timer_net_hb)>TIME_INTERVAL_REQ && message_type(msg)==MSG_TYPE_REQ){
		//get the req message 
		on_req_failed(msg);
		//remove the req message from the queue
		message_queue_flush(msg_q_tx, 1);
	    }
	}
	pthread_mutex_unlock(&mut_msg_tx);

	//heart beat packet
	if(difftime(timer, timer_net_hb)>TIME_INTERVAL_HB){
	    message_create(msg, MSG_NET_HB);
	    pthread_mutex_lock(&mut_msg_tx);
	    message_queue_put(msg_q_tx, msg);//send the hb to the server
	    timer_net_hb = time();//refresh the timer
	    pthread_mutex_unlock(&mut_msg_tx);
	}
	   
    }
}

//message handle function
int handle_msg(struct_message *msg){
    int req_num;
    int result = -1;
    switch(msg->data_type){
	case DATA_TYPE_STAT: 
	    //update stat
	    //sync to server
	    
	    result = 0;
	    break;
	case DATA_TYPE_CTRL:
	    //send ctrl to znet
	    msg2bytes(msg, write_serial); 	   
	    pthread_create(thrd_serial_tx, run_serial_tx);
	    pthread_join(thrd_serial_tx);
	    memset(write_serial, 0, BUFF_RW_LEN);
	    result = 0;
	    break;
	case DATA_TYPE_REQ_AUTH:
	    req_num = 0;
	    msg_stamp ++;
	    msg->stamp = msg_stamp;//add a stamp for the message
	    //send req
	    bytes2msg(bytes, msg); 
	    //wait 1 second
	    pthread_create(thrd_inet_client_tx, run_inet_client_tx, bytes);
	    pthread_join(thrd_inet_cient_tx);
	    sleep(1);
	    //check if ack msg is in the queue
	    pthread_mutex_lock(&mut_msg_handler);
	    if(message_queue_has_ack(msg_queue, msg->stamp)){
		message_queue_flush_stamp(msg_queue, msg->stamp);//flush all msg with the  same stamp
		result = 0; //if acked, return 0
		break;
	    }
	    pthread_mutex_unlock(&mut_msg_handler);
	    free(bytes);
	    break;
	case DATA_TYPE_NET_HB:
	    result = 0;
	    //do nothing for the ack
	    break;
    }
    return result;
}
