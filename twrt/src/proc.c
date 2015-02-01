#include "proc.h"

void message_copy(struct_message *msg_dst, struct_message *msg_src){
    message_flush(msg_dst->data);
    memcpy(msg_dst, msg_src, sizeof(struct_message);
    msg_dst->data = realloc(msg_dst->data, sizeof(char)*msg_src->data_len);
    memcpy(msg_dst->data, msg_src->data, sizeof(char)*msg_src->data_len);
}

void message_flush(struct_message *msg){//flush message as empty message
    memset(msg->data, 0, msg->data_len);
    free(msg->data);
    memset(msg, 0, sizeof(msg));
}

struct_message* message_create(struct_message* msg, int type, long stamp){
    message_flush(msg);
    switch(type){
	case MSG_REQ_NET_HB:
	    openwrt_get_id(wrt_get_id(msg->gateway_id));		   
	    msg->data_type = DATA_TYPE_NET_HB;
	    msg->stamp = stamp+1;
	    break;
	default:
	    break;
    }
}

int message_is_ack(struct_message* msg){
    if(msg->stamp!=0)
	return 1;
    else
	return 0;
}

void message_queue_init(struct_message_queu* msg_queue){
    msg_queue->msg = NULL;
    msg_queue->prev = msg_queue;
    msg_queue->next = msg_queue;
}

void message_queue_put(struct_message_queue** msg_q, struct_message* msg){
    message_queue *msg_q_new;
    if((*msg_q)->msg == NULL){//empty queue
	(*msg_q)->msg = msg;
    }else{  //msg in the queue
	msg_q_new = malloc(sizeof(message_queue));
	message_copy(msg_q_new->msg, msg);
	msg_q_new->prev = *msg_q;
	(*msg_q)->next = msg_q_new;
	msg_q_new->next = msg_q_new;
	*msg_q = msg_q_new;
    }
}

void message_queue_get(struct_message_queue* msg_q, struct_message* msg){
    message_queue *msg_q_item;
    msg_q_item = msg_q;
    while(msg_q_item->prev != msg_q_item)
	msg_queue_item = msg_q_item->prev;
    message_copy(msg, msg_q_item->msg);
    message_queue_flush(msg_q, 1); 
}

void message_queue_flush(struct_message_queue* msg_q, int len){
    int m;
    len = len>message_queue_getlen(msg_q)?message_queue_getlen(msg_q):len;
    message_queue *msg_q_item = msg_q;
    message_queue *msg_q_item2;

    while(msg_q_item->prev != msg_q_item)
	msg_q_item = msg_q_item->prev; //flush from the head

    
    for(m = 0; m<len; len++){
	//if queue has only one item
	if(message_queue_getlen(msg_q)<=1){
	    free(msg_q->msg);
	    msg_q->msg = NULL;
	    return;
	}
	msg_q_item->next->prev = msg_q_item->next;
	msg_q_item2 = msg_q_item;
	msg_q_item = msg_q_item->next;
	free(msg_q_item2->msg);
	free(msg_q_item2);
    }
}

void message_queue_flush_stamp(struct_message_queue** msg_q, long stamp){
    message_queue* msg_q_item = *msg_q;
    message_queue* msg_q_item2;
    if(message_queue_isempty(*msg_q))
	return;

    while(msg_q_item->prev != msg_q_item)
	msg_q_item = msg_q_item->prev;//flush from the head
    
    while(msg_q_item->next != msg_q_item){
	if(msq_q_item->msg->stamp == stamp){
	    //flush the msg and connect previous with next
	    if(msg_q_item->prev == msg_q_item){//if at the head
		msg_q_item2 = msg_q_item;
		msg_q_item = msg_q_item->next;
		msg_q_item->prev = msg_q_item;
		free(msg_q_item2->msg);
		free(msg_q_item2);
	    }else{
		msg_q_item2 = msg_q_item;
		msg_q_item = msg_q_item->next;
		msg_q_item->prev = msg_q_item2->prev;
		msg_q_item2->prev->next = msg_q_item;
		free(msg_q_item2->msg);
		free(msg_q_item2);
	    }
	}else{
	    msg_q_item = msg_q_item->next;
	}
    }
    if(msg_q_item->msg->stamp == stamp){//check the tail
	*msg_q = msg_q_item->prev;
	*msg_q->next = *msg_q;
	free(msg_q_item->msg);
	free(msg_q_item);
    }
}

int message_queue_has_stamp(struct_message_queue* msg_q, long stamp){
    int result = 0;
    if(message_queue_isempty(msg_q))
	    return 0; //if empty, return false
    while(msg_q->prev != msg_q)
	msg_q = msg_q->prev;//search from the head
    while(msg_q->next != msg_q){
	if(msg_q->msg->stamp == stamp){
	    result = 1;
	    return result;
	}
	msg_q = msg_q->next;
    }
    if(msg_q->msg->stamp == stamp)//check the tail
	result = 1;
    return result;
}

int message_queue_getlen(struct_message_queue* msg_q){
    struct_message_queue* msg_q_item = msg_q;
    int len=0;
    while(msg_q_item->next !=msg_q_item)
	msg_q_item=msg_q_item->next;

    if(msg_q->next == msg_q && msg_q->prev == msg_q){
	if(msg_q->msg == NULL)
	    return 0;
	if(msg_q->msg != NULL)
	    return 1;
    }
    else{
	while(msg_q_item->prev !=msg_q_item){
	    len ++;
	    msg_q_item = msg_q_item->prev;
	}
	len ++;
	return len;
    }
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
    int data_len;
    char *read_bytes = realloc(read_bytes, 4);
    if(len_buffer_byte_ring(bytes)<PROC_MSG_MIN)
	return;
    //locate the header
    read_buffer_byte_ring(bytes, read_bytes, 4);
    while(!(len_buffer_byte_ring(bytes)>=PROC_MSG_MIN && !memcmp(read_bytes, PROC_DATA_HEADER, 4))){
	get_buffer_byte_ring(bytes, read_bytes);//flush one byte until we get the header
	read_buffer_byte_ring(bytes, read_bytes, 4);
    }
    read_buffer_byte_ring(bytes, read_bytes, PROC_MSG_MIN-1);	  
    data_len = (int) (read_bytes[PROC_MSG_MIN-3] && 0x0Fh) + (int) (read_bytes[PROC_MSG_MIN-2]>>8 && 0xF0h);

    message_flush(msg);
    msg->data = realloc(msg->data, sizeof(char)*len);
    //the rest of the msg setting:w
    return len;
}
int msg2bytes(struct_message* msg, char* bytes){
    int len;
    return len;
}
