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

    sys_init(sys);
    inet_stat = 0;

    serial_open(&serial);
    while(inet_client_connect(&client) < 0)
	sleep(1); //sleep 1 second and try again
    inet_stat = 1;

    //3. retrieve authorization
    /////////////////////////////////////
    if(sys_get_auth(sys, client.fd)<0){
	perror("Device not authorized, please check your license\n");
	return -1;
    }
    //5. retrieve stamps from the web
    /////////////////////////////////////
    sys->stamp = sys_get_stamp(sys, &client);

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
	if(inet_stat){
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
}

void *run_client_rx(){
    int len;
    while(1){
	usleep(1000);
	if(inet_stat){
	    pthread_mutex_lock(&mut_client);
	    len = read(client->fd, read_client, INET_BUFF_LEN);//non-blocking reading, return immediately
	    if(len>0){
		put_buffer_byte_ring(buff_client, read_client, len);
		pthread_cond_signal(&cond_client);
	    }else if(errno == EAGAIN | errno == EINTR ){
		//continue
	    }else if(len == 0){//if disconnected, reconnect
		inet_stat = 0;//stop the thread 
		on_inet_client_disconnect();
	    }else{
		inet_stat = 0;//stop the thread 
		on_inet_client_disconnect();
	    }
	    pthread_mutex_unlock(&mut_client);
	}
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
	    message_destroy(msg);
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
	    message_destroy(msg);
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
	if(!message_queue_isempty(msg_q_rx))
	    msg = message_queue_get(msg_q_rx, msg);
	pthread_mutex_unlock(&mut_msg_rx);//unlock msg_q_rx first

	handle_msg(msg);
	message_destroy(msg);
    }
}

void *run_sys_msg_tx(){
    struct_message *msg;
    struct_message_queue msg_q_item = msg_q_tx;
    char val_zero[16];
    memset(val_zero, 0, 16);
    while(1){
	usleep(1000);
	pthread_mutex_lock(&mut_msg_tx);
	if(message_queue_isempty(msg_q_tx)){
	    pthread_mutex_unlock(&mut_msg_tx);
	    continue;
	}
	while(msg_q_item>prev != msg_q_item)
	    msg_q_item = msg_q_item->prev;//move the the head of the queue
	while(msg_q_item->next != msg_q_item)
	    if(message_is_req(msg_q_item->msg))//if is req msg
		if(!memcmp(msg_q_item->msg->timer, val_zero, sizeof(time_t))){//unsend req
		    msg_q_item->msg->timer = time();
		    break;			
		}else
		    msg_q_item = msg_q_item->next;
	    else
		break;	

	if(msg_q_item->next == msg_q_item)//check the tail
	    if(message_is_req(msg_q_item->msg))//if is req msg
		if(!memcmp(msg_q_item->msg->timer, val_zero, sizeof(time_t)))//unsend req
		    msg_q_item->msg->timer = time();
		else
		    continue; //if tail is send req msg, continue the loop

	char* bytes;
	int len = msg2bytes(msg_q_item->msg, bytes);
	//skipped all send req msg and locate the first unsend msg
	//identify the write direction
	switch(message_direction(msg)){
	    case MSG_TO_SERIAL:
		pthread_create(thrd_serial_tx, run_serial_tx, msg);
		pthread_join(thrd_serial_tx);
		break;
	    case MSG_TO_INET_CLIENT:
		pthread_create(thrd_client_tx, run_serial_tx, msg);
		pthread_join(thrd_client_tx);
		break;
	    //case MSG_TO_INET_SERVER:
		//pthread_create(thrd_client_tx, run_serial_tx, msg);
		//pthread_join(thrd_client_tx);
		//break;
	    default://if unknown, flush the message from the queue
		    message_queue_del(msg_q_item);
		break;
	}
	free(bytes);
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
    char* byte_val;
    int val;
    int m;
    long val2;
    int result = -1;
    switch(msg->data_type){
	case DATA_TYPE_STAT: 
	    //update stat
	    val = sys_znode_update(sys, msg);
	    //sync to server
	    sys_sync_znode(sys, val, &msg_q_tx);
	    result = 0;
	    break;
	case DATA_TYPE_CTRL:
	    //send ctrl to znet
	    message_queue_put(&msg_q_tx, msg);
	    result = 0;
	    break;
	case DATA_TYPE_NET_HB:
	    message_queue_put(&msg_q_tx, msg);
	    result = 0;
	    //do nothing for the ack
	    break;
	case DATA_TYPE_AUTH_ACK:
	    memcpy(sys->cookie, msg->data, SYS_COOKIE_LEN);
	    pthread_mutext_lock(&mut_msg_tx);
	    message_queue_del_stamp(msg_q_tx, msg->stamp);
	    pthread_mutext_unlock(&mut_msg_tx);
	    result = 0;
	    break;
	case DATA_TYPE_HB_ACK:
	    pthread_mutext_lock(&mut_msg_tx);
	    message_queue_del_stamp(msg_q_tx, msg->stamp);
	    pthread_mutext_unlock(&mut_msg_tx);
	    result = 0;
	    break;
	case DATA_TYPE_SYNC_STAT:
	    for(m = 0; m<sys->znode_num; m++)
		if(!memcmp(sys->znode_list[m].id, msg->dev_id, 6)){
		    val = m;
		    break;
		}
	    memcpy(val2, msg->data+data_len-4, 4);
	    if(val2>sys->znode_list[m]->u_stamp)
		memcpy(sys->znode_list[m]->status, msg->data, msg->data_len-4);
	    sys->znode_list[m]->status_znet = ZNET_ON;
	    result = 0;
	    break;
	default:
	    break;
    }
    return result;
}

void on_inet_client_disconnect(){
    //when disconnected, stop all threads and flush all queue
    int m;
    pthread_mutex_lock(&mut_msg_rx);
    pthread_mutex_lock(&mut_msg_tx);
    
    while(inet_client_connect(&client) < 0)
	sleep(1); //sleep 1 second and try again
    inet_stat = 1;

    if(sys_get_auth(sys, client.fd)<0){
	perror("Device not authorized, please check your license\n");
	return -1;
    }
    sys->u_stamp = sys_get_stamp(sys, &client);
    for(m = 0; m<sys->znode_num; m++){
	sys->znode[m]->u_stamp = sys->u_stamp;
	sys->znode[m]->g_stamp = sys->g_stamp;
    }

    pthread_mutex_unlock(&mut_msg_rx);
    pthread_mutex_unlock(&mut_msg_tx);

    //reconnect
    //send lic/cookie
    //get ack from the server
    //restart all threads
}
