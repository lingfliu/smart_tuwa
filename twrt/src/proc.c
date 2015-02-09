#include "proc.h"


struct_message* message_copy(struct_message *msg_dst, struct_message *msg_src){
    message_flush(msg_dst);
    memcpy(msg_dst, msg_src, sizeof(struct_message));
    msg->data = NULL;
    msg_dst->data = realloc(msg_dst->data, sizeof(char)*msg_src->data_len);
    memcpy(msg_dst->data, msg_src->data, sizeof(char)*msg_src->data_len);
    return msg_dst;
}

struct_message* message_destroy(struct_message *msg){//delete message
    memset(msg->data, 0, msg->data_len);
    free(msg->data);
    msg->data = NULL;
    memset(msg, 0, sizeof(*msg));
    free(msg);
    return msg;
}

void message_flush(struct_message *msg){//flush message 
    memset(msg->data, 0, msg->data_len);
    free(msg->data);
    memset(msg, 0, sizeof(struct_message));
}

int message_is_req(struct_message* msg){
    switch(msg->data_type){
	case DATA_TYPE_REQ_AUTH:
	    return 1;
	case DATA_TYPE_NET_HB:
	    return 1;
	default:
	    return 0;
    }
}

///////////////////////////////////////////////////
//message queue functions

void message_queue_init(struct_message_queu* msg_q){
    msg_queue->msg = NULL;
    msg_q->prev = msg_q;
    msg_q->next = msg_q;
}

void message_queue_put(struct_message_queue** msg_q, struct_message* msg){
    message_queue *msg_q_item;
    if(message_queue_isempty(*msg_q)){//empty queue
	(*msg_q)->msg = message_copy((*msg_q)->msg, msg);
    }else{  //queue has msgs
	msg_q_item = realloc(sizeof(message_queue));
	msg_q_item->msg = message_copy(msg_q_item->msg, msg);
	msg_q_item->prev = *msg_q;
	(*msg_q)->next = msg_q_item;
	msg_q_item->next = msg_q_item;
	*msg_q = msg_q_item; //update the position of the msg_q
    }
}

struct_message* message_queue_get(struct_message_queue** msg_q, struct_message* msg){
    msg_q_item = *msg_q;
    if((*msg_q)->msg==NULL)
	return msg;//do nothing
    msg = message_copy(msg, msg_q_item->msg);
    message_queue_del(msg_q);
    return msg;
}

void message_queue_del(struct_message_queue** msg_q){
    struct_message_queue* msg_q_item = *msg_q;
    if(message_queue_istempty(msg_q_item))//empty queue, do nothing
	return;
    msg_q_item->msg = message_destroy(msg_q_item->msg);//destroy the message
    if(msg_q_item->prev == msg_q_item && msg_q_item->next = msg_q_item){//one msg in the queue, only destroy the message
	return;
    }
    if(msg_q_item->prev == msg_q_item){ //head
	free(msg_q_item);
	return;
    }
    if(msg_q_item->next == msg_q_item){ //tail
	*msg_q = msg_q_item->prev;
	free(msg_q_item);
	return;
    }

    //in the middle
    msg_q_item->prev->next = msg_q_item->next;
    msg_q_item->next->prev = msg_q_item->prev;
}

int message_queue_del_stamp(struct_message_queue** msg_q, long stamp){
    int has_stamp = -1;
    message_queue* msg_q_item = *msg_q;
    if(message_queue_isempty(*msg_q))
	return has_stamp;

    while(msg_q_item->prev != msg_q_item)
	msg_q_item = msg_q_item->prev;//move to the head
    
    while(msg_q_item->next != msg_q_item){
	if(msq_q_item->msg->stamp == stamp){
	    message_queue_del(&msg_q_item);
	    has_stamp = 0;
	}
	msg_q_item = msg_q_item->next;
    }

    if(msg_q_item->msg->stamp == stamp){//check the tail
	message_queue_del(&msg_q_item);
	has_stamp = 0;
    }
    return has_stamp;
}

int message_queue_has_stamp(struct_message_queue* msg_q, long stamp){
    int has_stamp = 0;
    if(message_queue_isempty(msg_q))
	return has_stamp;

    while(msg_q->prev != msg_q)
	msg_q= msg_q->prev;//move to the head
    
    while(msg_q->next != msg_q){
	if(msq_q->msg->stamp == stamp){
	    has_stamp = 1;
	}
	msg_q= msg_q->next;
    }

    if(msg_q->msg->stamp == stamp){//check the tail
	has_stamp = 1;
    }
    return has_stamp;
}

int message_queue_len(struct_message_queue* msg_q){
    int len=0;
    if(msg_q->next == msg_q && msg_q->prev == msg_q){
	if(msg_q->msg == NULL)
	    return 0;
	if(msg_q->msg != NULL)
	    return 1;
    }

    while(msg_q->next != msg_q)
	msg_q=msg_q->next;
    
    while(msg_q->prev != msg_q){
	len ++;
	msg_q_item = msg_q_item->prev;
    }
    len ++;
    return len;
}

int message_queue_isempty(struct_message_queue* msg_q){
    if(msg_q->next == msg_q && msg_q->prev == msg_q && msg_q->msg == NULL)
	return 1;
    else
	return 0;
}

//transfer bytes into one message from the beginning
int bytes2msg(buffer_byte_ring* bytes, struct_message* msg){
    int len;
    char* read_bytes;

    if(buffer_byte_ring_len(bytes)<PROC_MSG_MIN)
	return;//if not sufficient for a msg, return

    //locate the header
    read_bytes = realloc(read_bytes, 4);
    buffer_byte_ring_read(bytes, read_bytes, 4);
    while(!memcmp(read_bytes, PROC_DATA_HEADER, 4)){
	buffer_byte_ring_get(bytes, NULL, 1); //remove one byte
	if(buffer_byte_ring_len(bytes)<PROC_MSG_MIN)
	   return 0; 
    }
    if(buffer_byte_ring_len(bytes)<PROC_MSG_MIN)
	return 0; 
    else{
	read_bytes = realloc(read_bytes, 18);
	buffer_byte_ring_read(bytes, read_bytes, 18);
	len = read_bytes[16]>>8+read_bytes[17] + 18;
	if(buffer_byte_ring_len(bytes)<len)
	    return 0;
	else{
	    read_bytes = realloc(read_bytes, len);
	    memcpy(msg->gateway_id, *bytes, 6);
	    memcpy(msg->dev_id, *bytes+6, 6);
	    memcpy(msg->dev_type, *bytes+12, 2);
	    memcpy(msg->data_type, *bytes+14, 2);
	    memcpy(msg->data_len, *bytes+16, 2);
	    msg->data = realloc(msg->data, len-18);
	    memcpy(msg->data, *bytes+18, msg->data_len);
	    memcpy(msg->stamp, *bytes+18+msg->data_len, 4);
	    free(read_bytes);
	    buffer_byte_ring_get(bytes, NULL, len);
	    return 1;
	}
    }
}
int msg2bytes(struct_message* msg, char** bytes){
    int len;
    len = 22+msg->data_len;
    *bytes = realloc(*bytes, len); 
    memcpy(*bytes,msg->gateway_id, 6);
    memcpy(*bytes+6, msg->dev_id, 6);
    memcpy(*bytes+12, msg->dev_type, 2);
    memcpy(*bytes+14, msg->data_type, 2);
    memcpy(*bytes+16, msg->data_len, 2);
    memcpy(*bytes+18, msg->data, msg->data_len);
    memcpy(*bytes+18+msg->data_len, msg->stamp, 4);

    return len;
}
