#include "proc.h"

void copy_message(struct_message *msg_dst, struct_message *msg_src){
    msg_dst->data = malloc(sizeof(char)*msg_src->data_len);
    memcpy(msg_dst->data, msg_src->data, sizeof(char)*msg_src->data_len);
    memcpy(msg_dst->id_gateway, msg_src->id_gateway, 6);
    memcpy(msg_dst->id_dev, msg_src->id_dev, 6);
    msg_dst->dev_type = msg_src->dev_type;
    msg_dst->data_type = msg_src->data_type;
    msg_dst->data_len = msg_src->data_len;
}

void flush_message(struct_message *msg){//flush message as empty message
    free(msg->data);
    memset(msg, 0, sizeof(msg));
}

void init_message_queue(struct_message_queu* msg_queue){
    msg_queue->msg = NULL;
    msg_queue->prev = msg_queue;
    msg_queue->next = msg_queue;
}

void put_message_queue(struct_message_queue* msg_queue, struct_message* msg){
    message_queue *msg_queue_new;
    if(msg_queue->msg == NULL){//empty queue
	msg_queue->msg = msg;
    }else{  //msg in the queue
	msg_queue_new = malloc(sizeof(message_queue));
	msg_queue_new->msg = msg;
	msg_queue_new->prev = msg_queue;
	msg_queue->next = msg_queue_new;
	msg_queue_new->next = msg_queue_new;
	msg_queue = msg_queue_new;
    }
}

void get_message_queue(struct_message_queue* msg_queue, struct_message* msg){
    message_queue *msg_queue_inquire;
    msg_queue_inquire = msg_queue;
    while(msg_queue_inquire->prev != msg_queue_inquire)
	msg_queue_inquire = msg_queue_inquire->prev;
    copy_message(msg, msg_queue_inquire->msg);
    flush_message_queue_head(msg_queue, 1); 
}

void flush_message_queue(struct_message_queue* msg_queue, int len){
    int m;
    message_queue *msg_queue_inquire;
    message_queue *msg_queue_inquire_2;
    msg_queue_inquire = msg_queue;
    while(msg_queue_inqurie->prev != msg_queue_inquire)
	msg_queue_inquire = msg_queue_inquire->prev;
    msg_queue_inquire->next->prev = msg_queue_inquire->next;

    for(m = 0; m<len; len++){
	msg_queue_inquire_2 = msg_queue_inquire;
	msg_queue_inquire = msg_queue_inquire->next;
	msg_queue_inquire_2->next->prev = msg_queue_inquire_2->next;
	flush_message_queue_single(msg_queue_inquire_2);
    }
}

void flush_message_queue_single(struct_message_queue* msg_queue){
    flush_message(msg_queue->msg);
    free(msg_queue->msg);
    free(msg_queue);
}
