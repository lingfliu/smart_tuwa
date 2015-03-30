#include "proc.h"

message* message_create(){
   message *msg = calloc(sizeof(message));
   return msg;
}

void message_destroy(message *msg){
    memset(msg->data, 0, msg->data_len);
    free(msg->data);
    memset(msg,0,sizeof(message));
    free(msg);
}

void message_flush(message *msg){//flush message without destroying it
    memset(msg->data, 0, msg->data_len);
    free(msg->data);
    memset(msg, 0, sizeof(message));
}

void message_copy(message *msg_dst, message *msg_src){
    message_flush(msg_dst);
    memcpy(msg_dst, msg_src, sizeof(message));
    msg->data = NULL;
    msg_dst->data = realloc(msg_dst->data, sizeof(char)*msg_src->data_len);
    memcpy(msg_dst->data, msg_src->data, sizeof(char)*msg_src->data_len);
}

int message_isreq(message *msg){
    switch(msg->data_type){
	case DATA_REQ_SYNC:
	    return 1;
	case DATA_REQ_AUTH_GW:
	    return 1;
	case DATA_REQ_AUTH_DEV:
	    return 1;
	case DATA_REQ_STAMP:
	    return 1;
	case DATA_REQ_PULSE:
	    return 1;
	default:
	    return 0;
    }
    return isreq;
}

int message_tx_dest(message* msg){ //get tx message destination
    switch(msg->data_type){
	case DATA_STAT: 
	    return MSG_TO_SERVER;
	case DATA_CTRL: 
	    return MSG_TO_ZNET;
	case DATA_REQ_SYNC:
	    return MSG_TO_SERVER;
	case DATA_REQ_AUTH_GW:
	    return MSG_TO_SERVER;
	case DATA_REQ_AUTH_DEV:
	    return MSG_TO_SERVER;
	case DATA_REQ_USER:
	    return MSG_TO_SERVER;
	case DATA_REQ_STAMP:
	    return MSG_TO_SERVER;
	case DATA_REQ_PULSE:
	    return MSG_TO_SERVER;
	default:
	    return MSG_TO_SERVER;
    }
}

//transfer bytes into one message from the beginning
int bytes2message(buffer_ring_byte* bytes, message* msg){
    int data_len;
    char pre_bytes[50];
    char *data;

    if(buffer_ring_byte_len(bytes)<MSG_LEN_MIN)
	return 0;//if not sufficient for a msg, return

    //locate the header
    buffer_ring_byte_read(bytes, pre_bytes, MSG_LEN_HEADER_GW);
    while(!memcmp(pre_bytes, MSG_HEADER_GW, 4)){//locate the header
	buffer_ring_byte_get(bytes, NULL, 1); //remove one byte
	if(buffer_ring_byte_len(bytes)<MSG_LEN_MIN)
	   return 0; 
	buffer_ring_ring_read(bytes, pre_bytes, MSG_LEN_HEADER_GW);
    }
    if(buffer_ring_byte_len(bytes)<MSG_LEN_MIN)
	return 0;  //header found, but lenght is not long enough
    else{
	buffer_byte_ring_read(bytes, pre_bytes, MSG_LEN_FIXED); //read the fixed length 
	memcpy(data_len, pre_bytes+MSG_POS_DATA_LEN, sizeof(int)); //get data length
	if(buffer_ring_byte_len(bytes)<data_len+MSG_LEN_FIXED) //if all data in the bytes
	    return 0;
	else{ //start read the data
	    buffer_ring_byte_get(bytes, pre_bytes, MSG_LEN_FIXED);
	    
	    data = calloc(data_len, sizeof(char));
	    buffer_ring_byte_get(bytes, data, data_len);

	    memcpy(msg->stamp, *pre_bytes+MSG_POS_STAMP, MSG_LEN_STAMP);
	    memcpy(msg->gateway_id, *pre_bytes+MSG_POS_ID_GW, MSG_LEN_ID_GW);
	    memcpy(msg->dev_id, *pre_bytes+MSG_POS_ID_DEV, MSG_LEN_ID_DEV);
	    memcpy(msg->dev_type, *bytes+MSG_POS_DEV_TYPE, MSG_LEN_DEV_TYPE);
	    memcpy(msg->data_type, *bytes+MSG_POS_DATA_TYPE, MSG_LEN_DATA_TYPE);
	    memcpy(msg->data_len, *bytes+MSG_POS_DATA_LEN, MSG_LEN_DATA_LEN);
	    msg->data = realloc(msg->data, data_len);
	    memcpy(msg->data, *data, data_len);

	    free(data);//free the temp data buffer
	    return MSG_LEN_FIXED+data_len;
	}
    }
}

int message2bytes(message* msg, char** bytes){
    int len;
    len = MSG_LEN_FIXED+msg->data_len;
    *bytes = realloc(*bytes, len); 

    //conver the prefix
    memcpy(*bytes, MSG_HEADER_GW, MSG_LEN_HEADER_GW);
    memcpy(*bytes+MSG_POS_STAMP, msg->stamp, MSG_LEN_STAMP);
    memcpy(*bytes+MSG_POS_ID_GW, msg->gateway_id, MSG_LEN_ID_GW);
    memcpy(*bytes+MSG_POS_ID_DEV, msg->dev_id, MSG_LEN_ID_DEV);
    memcpy(*bytes+MSG_POS_DEV_TYPE, msg->dev_type, MSG_LEN_DEV_TYPE);
    memcpy(*bytes+MSG_POS_DATA_TYPE, msg->data_type, MSG_LEN_DATA_TYPE);
    memcpy(*bytes+MSG_POS_DATA_LEN, msg->data_len, MSG_LEN_DATA_LEN);

    memcpy(*bytes+MSG_LEN_FIXED, msg->data, msg->data_len); //conver the data

    return len;
}

///////////////////////////////////////////////////
//message queue functions
message_queue* message_queue_create(){
    return calloc(sizeof(message_queue));
}

message_queue* message_queue_flush(message_queu* msg_q){
    //move to the head
    msg_q = message_queue_to_head(msg_q);
    message_queue *msg_q_item;
    while(msg_q->next != msg_q){
	msg_q_item = msg_q;
	msg_q = msg_q->next;//move to next msg
	msg_q->prev = msg_q; //set the next msg as the head
	message_destroy(msg_q_item->msg); //delete current msg
	free(msg_q_item); //free current msg_q
    }
    message_destroy(msg_q->msg); //keep the last item in the queue
    return msg_q;
}

void message_queue_destroy(message_queu* msg_q){
    msg_q = message_queue_flush(msg_q);
    free(msg_q);
}

void message_queue_init(message_queue *msg_q){
    memset(msg_q, sizeof(message_queue));
    msg_queue->msg = NULL;
    msg_q->prev = msg_q;
    msg_q->next = msg_q;
}

message_queue* message_queue_put(message_queue* msg_q, message* msg){
    if(message_queue_getlen(msg_q)<=0){
	message_copy(msg_q->msg, msg); //empty queue
    }else{
	msg_q->next = message_queue_create();
	msg_q->next->prev = msg_q;
	msg_q = msg_q->next;
	msg_q->next = msg_q;
	message_copy(msg_q->msg, msg);
    }
    return msg_q;
}

message_queue* message_queue_get(message_queue* msg_q, message* msg){
    message_queue msg_q_item;
    int len = message_queue_getlen(msg_q);
    if(len <= 0){
	msg = NULL;
    }else if(len == 1){
	message_copy(msg, msg_q->msg);
	message_destroy(msg_q->msg);
    }else{
	message_copy(msg,msg_q->msg);
	message_destroy(msg_q->msg);
	msg_q_item = msg_q;
	msg_q = msg_q->next; //move to the next item 
	msg_q->prev = msg_q;
	free(msg_q_item); //delete the current item
    }
    return msg_q;
}

int message_queue_del(message_queue **msg_q){
    message_queue *msg_q_item;
    if(*msg_q->msg == NULL && msg_q->prev == *msg_q->next) //if is empty queue
	return 0;
    else{
	message_destroy(*msg_q->msg); //delete the message

	//then delete the message queue item
	if(*msg_q->prev == *msg_q && *msg_q->next == *msg_q) //one message in the queue
	else if(*msg_q->prev == *msg_q){ //head
	    msg_q_item = *msg_q;
	    *msg_q = *msg_q->next;
	    *msg_q->prev = *msg_q;
	    free(msg_q_item);
	}else if(*msg_q->next == *msg_q){ //tail
	    msg_q_item = *msg_q;
	    *msg_q = *msg_q->prev;
	    *msg_q->next = *msg_q;
	    free(msg_q_item);
	}else{ //in the middle
	    msg_q_item = *msg_q;
	    *msg_q = *msg_q->next;
	    *msg_q->prev = msg_q_item->prev;
	    msg_q_item->prev->next = *msg_q;
	    free(msg_q_item);
	}
	return 1;//return the number of deletion
}

int message_queue_del_stamp(message_queue **msg_q_p, int stamp){
    message_queue *msg_q = *msg_q_p;
    message_queue *msg_q_item;
    msg_q = message_queue_to_head(msg_q);
    
    int cnt = 0;
    len = message_queue_getlen(msg_q);

    if(len <=0){ //no msg
	return 0;
    }else if(len==1){ //one msg
	if(msg_q->msg->stamp == stamp){
	    cnt++;
	    message_destroy(msg_q->msg); //remove the message
	    memset(msg_q->time, 0, sizeof(timeval));//reset the time
	}
	return cnt;
    }else{
	while(msg_q->next != msg_q){ //if not the last one
	    if(msg_q->msg->stamp == stamp){
		cnt++;
		message_destroy(msg_q->msg);
		msg_q_item = msg_q;

		if(msg_q->prev == msg_q) //if the first one
		    msg_q->next->prev = msg_q->next; //replace the head with next
		else{
		    //connect the adjscent msg
		    msg_q->prev->next = msg_q->next;
		    msg_q->next->prev = msg_q->prev;
		}

		msg_q = msg_q->next; //move to the next msg 
		free(msg_q_item);
	    }else{
		msg_q = msg_q->next; //check the next msg
	    }
	}

	//check the last item
	if(msg_q->prev == msg_q){//check if only one in the queue
	    if(msg_q->msg->stamp == stamp){
		cnt++;
		message_destroy(msg_q->msg); //remove the message
		memset(msg_q->time, 0, sizeof(timeval));//reset the time
	    }
	}else{
	    if(msg_q->msg->stamp == stamp){
		cnt++;
		mesage_destroy(msg_q->msg);
		msg_q_item = msg_q;
		msg_q = msg_q->prev;
		msg_q->next = msg_q;
		free(msg_q_item);
	    }
	}
	*msg_q_p = msg_q; //update the msg_q_p
	return cnt;
    }
}

int message_queue_getlen(message_queue* msg_q){
    msg_q = message_queue_to_head(msg_q);
    int len=0;
    if(msg_q->next == msg_q && msg_q->prev == msg_q){
	if(msg_q->msg == NULL)
	    return 0;
	if(msg_q->msg != NULL)
	    return 1;
    }

    len++;
    while(msg_q->next != msg_q){
	len++;
	msg_q=msg_q->next;
    }

    return len;
}

message_queue* message_queue_to_head(message_queue *msg_q){
    while(msg_q->prev != msg_q)
	msg_q = msg_q->prev;
    return msg_q;
}

message_queue* message_queue_to_tail(message_queue *msg_q){
    while(msg_q->next != msg_q)
	msg_q = msg_q->next;
    return msg_q;
}

message* message_create_stat(int stat_len, char* stat, char id_gw[8], char id_dev[8], long stamp){
    message *msg = message_create();
    msg->data = realloc(msg->data, stat_len);
    memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
    memcpy(msg->dev_id, id_dev, MSG_LEN_ID_DEV);
    msg->data_type = DATA_STAT;
    memcpy(msg->data, stat, stat_len);
    msg->data_len = stat_len;
    msg->stamp = stamp;
    return msg;
}

message* message_create_ctrl(int ctrl_len, char* ctrl, char id_gw[8], char id_dev[8], long stamp){
    message *msg = message_create();
    msg->data = realloc(msg->data, ctrl_len);
    memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
    memcpy(msg->dev_id, id_dev, MSG_LEN_ID_DEV);
    msg->data_type = DATA_CTRL;
    memcpy(msg->data, ctrl, ctrl_len);
    msg->data_len = ctrl_len;
    msg->stamp = stamp;
    return msg;
}

message* message_create_sync(int stat_len, char* stat, long u_stamp, char id_gw[8], char id_dev[8], long stamp){
    message *msg = message_create();
    msg->data = realloc(msg->data, stat_len+4);
    memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
    memcpy(msg->dev_id, id_dev, MSG_LEN_ID_DEV);
    msg->data_type = DATA_REQ_SYNC;
    memcpy(msg->data, &u_stamp, 4);
    memcpy(msg->data+4, ctrl, ctrl_len);
    msg->data_len = stat_len+4;
    msg->stamp = stamp;
    return msg;
}

message* message_create_auth_gw(int lic_len, char* lic, char id_gw[8], char id_dev[8], long stamp){
    message *msg = message_create();
    msg->data = realloc(msg->data, lic_len);
    memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
    memcpy(msg->dev_id, id_dev, MSG_LEN_ID_DEV);
    msg->data_type = DATA_REQ_AUTH_GW;
    memcpy(msg->data, lic, lic_len);
    msg->data_len = lic_len;
    msg->stamp = stamp;
    return msg;
}

message* message_create_auth_dev(char id_gw[8], char id_dev[8], long stamp){
    message *msg = message_create();
    msg->data = realloc(msg->data, 1);
    memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
    memcpy(msg->dev_id, id_dev, MSG_LEN_ID_DEV);
    msg->data_type = DATA_REQ_AUTH_DEV;
    memset(msg->data, 0, 1);
    msg->data_len = 1;
    msg->stamp = stamp;
    return msg;
}

message* message_create_pulse(char id_gw[8], long stamp){
    message *msg = message_create();
    msg->data = realloc(msg->data, 1);
    memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
    memcpy(msg->dev_id, id_dev, MSG_LEN_ID_DEV);
    msg->data_type = DATA_REQ_PULSE;
    memset(msg->data, 0, 1);
    msg->data_len = 1;
    msg->stamp = stamp;
    return msg;
}

message* message_create_req_stamp(char id_gw[8]){
    message *msg = message_create();
    msg->data = realloc(msg->data, 1);
    memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
    memcpy(msg->dev_id, NULL_DEV, MSG_LEN_ID_DEV);
    msg->data_type = DATA_REQ_STAMP;
    memset(msg->data, 0, 1);
    msg->data_len = 1;
    msg->stamp = 0;
    return msg;
}

message* message_create_req_user(char id_gw[8], char id_user[8], long stamp){
    message msg* = message_create();
    msg->data = realloc(msg->data, 1);
    msg->data_len = 1;
    return msg;
}
