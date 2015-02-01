#include "twrt.h"

int main(int argn, char* argv[]){
    //1. reset the config
    //2. check the network
    //3. retrieve authorization
    //4. 
    //5. retrieve stamps
    //6. check the znet and retrieve the node status
    //7. synchronization with the web
    //8. start threads
    get_config(&cfg);
    serial_config(cfg->serial_name, cfg->serial_type, cfg->serial_baudrate, &serial);
    inet_client_config(cfg->server_ip, cfg->server_port, cfg->, &inet_client);

    //initialize buffers
    buffer_serial = create_buffer_byte_ring(BUFF_RING_LEN);
    buffer_inet_client = create_buffer_byte_ring(BUFF_RING_LEN);
    
    //initialize queue
    msg = malloc(sizeof(struct_message));
    msg->data = malloc(sizeof(char));
    message_queue_init(msg_queue);
    msg_stamp = 0; 


    pthread_mutex_init(&mut_serial);
    pthread_mutex_init(&mut_inet_client);
    pthread_mutex_init(&mut_msg_handler);
    pthread_cond_init(&cond_serial);
    pthread_cond_init(&cond_inet_client);
    //pthread_cond_init(&cond_msg_handler);

    pthread_create(thrd_serial_rx, run_serial_rx, NULL);
    pthread_create(thrd_inet_client_rx, run_inet_client, NULL);
    pthread_create(thrd_translate_serial, run_serial, NULL);
    pthread_create(thrd_translate_serial, run_translate_serial, NULL);
    pthread_create(thrd_translate_inet_client, run_translate_inet_client, NULL);
    pthread_create(thrd_msg_handler, run_msg_handler, NULL);
    
    pthread_create(thrd_sys, run_sys, NULL);

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

void *run_inet_client_rx(){
    int len;
    while(1){
	usleep(1000);
	pthread_mutex_lock(&mut_inet_client);
	len = read(int_client->fd, read_inet_client, INET_BUFF_LEN);//non-blocking reading, return immediately
	if(len>0){
	    put_buffer_byte_ring(buffer_inet_client, read_inet_client, len);
	    pthread_cond_signal(&cond_inet_client);
	}else if(errno == EAGAIN | errno == EINTR ){
	    //continue
	}else if(len == 0){//if disconnected, reconnect
	    while(inet_client_connect(inet_client)<0)
		sleep(1);// wait 1 s and try connect again
	}else{
	    while(inet_client_connect(inet_client)<0)
		sleep(1);// wait 1 s and try connect again
	}
	pthread_mutex_unlock(&mut_inet_client);
    }
}

void *run_translate_serial(){
    while(1){
	usleep(1000);
	pthread_mutex_lock(&mut_serial);
	pthread_cond_wait(&cond_serial, &mut_serial);
	//translate bytes into message
	while(bytes2msg(buff_serial, msg_serial)>0){
	    usleep(1);
	    pthread_mutex_lock(&mut_msg_handler);
	    message_queue_put(&msg_queue, msg_serial);
	    pthread_mutex_unlock(&mut_msg_handler);
	}
	pthread_mutex_unlock(&mut_serial);
    }
}

void *run_translate_inet_client(){
    while(1){
	usleep(1000);
	pthread_mutex_lock(&mut_inet_client);
	pthread_cond_wait(&cond_inet_client, &mut_inet_client);
	//translate bytes into message
	while(bytes2msg(buff_inet_client, msg_inet_client)>0){
	    usleep(1);
	    pthread_mutex_lock(&mut_msg_handler);
	    message_queue_put(&msg_queue, msg_inet_client);
	    pthread_mutex_unlock(&mut_msg_handler);
	}
	pthread_mutex_unlock(&mut_inet_client);
    }
}

void *run_msg_handler(){
    struct_message *msg;
    message_flush(msg);
    while(1){
	usleep(1000);
	pthread_mutex_lock(&mut_msg_handler);
	if(message_queue_isempty(msg_queue))
		continue;
	else
	    message_queue_get(msg_queue, msg);
	pthread_mutex_unlock(&mut_msg_handler);//unlock the msg_queue first
	if(msg != NULL){
	    handle_msg(msg);
	    message_flush(msg);
	}
    }
}

void* run_sys_ptask(){
    struct_message* msg;
    while(1){	
	//periodic tasks
	sleep(3);
	//send the hb to the server

	   
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
	case DATA_TYPE_ACK:
	    result = 0;
	    //do nothing for the ack
	    break;
    }
    return result;
}
