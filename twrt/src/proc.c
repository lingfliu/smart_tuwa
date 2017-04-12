#include "proc.h"


/************************************************
  message operation
 ************************************************/
message* message_create(){
	message *msg = calloc(sizeof(message), sizeof(char));
	msg->data = NULL; //create empty message
	return msg;
}

void message_destroy(message *msg){
	if(msg->data == NULL) //empty message do nothing
		free(msg);
	else{
		free(msg->data);
		free(msg);
	}

}

void message_flush(message *msg){//flush message without destroying it
	if(msg->data == NULL) //empty message do nothing
		memset(msg, 0, sizeof(message));
	else{
		free(msg->data);
		memset(msg, 0, sizeof(message));
		msg->data = NULL;
	}
}

void message_copy(message *msg_dst, message *msg_src){
	message_flush(msg_dst);
	
	memcpy(msg_dst, msg_src, sizeof(message));

	msg_dst->data = NULL; //reset data
	if(msg_src->data == NULL) //if src message is empty 
		return;
	else{
		msg_dst->data = calloc(sizeof(char)*msg_src->data_len, sizeof(char));
		memcpy(msg_dst->data, msg_src->data, sizeof(char)*msg_src->data_len);
	}
}

int message_isreq(message *msg){
	switch(msg->data_type){
		case DATA_REQ_SYNC:
			return 0;
		case DATA_REQ_AUTH_GW:
			return 1;
		case DATA_REQ_AUTH_DEV:
			return 1;
		case DATA_REQ_STAMP:
			return 1;
		case DATA_REQ_PULSE:
			return 0;
		case DATA_DEL_ZNODE:
			return 0;
		default:
			return 0;
	}
}

int message_tx_dest(message* msg){ //get tx message destination
	switch(msg->data_type){
		case DATA_STAT:
			return MSG_TO_SERVER;
		case DATA_CTRL: 
			return MSG_TO_ZNET;
		case DATA_PULSE:
			return MSG_TO_SERVER;
		case DATA_SYNC:
			return MSG_TO_LOCALUSER;
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
		case DATA_SET:
			return MSG_TO_SERVER;
		case DATA_ACK_AUTH_LOCAL:
			return MSG_TO_LOCALUSER;
		case DATA_NULL:
			return MSG_TO_SERVER;
		case DATA_INSTALL:
			return MSG_TO_SERVER;
		//case DATA_INSTALL_INFO:
		//	return MSG_TO_SERVER;
		case DATA_DEL_ZNODE:
			return MSG_TO_SERVER;
		case DATA_SCENE:
			return MSG_TO_SERVER;
		case DATA_ACK_INSTALL_OP:
			return MSG_TO_SERVER;
		case DATA_ACK_SCENE_OP:
			return MSG_TO_SERVER;
		default:
			return MSG_TO_SERVER; //unspecified message are not forwarded
	}
}

int message_isvalid(message *msg){
	return 1;
}

//transfer bytes into one message from the beginning
int bytes2message(buffer_ring_byte* bytes, message* msg){
	int data_len;
	char pre_bytes[50];
	char *data;
	int m;

	//if buffer is too short for a message, return 
	if(buffer_ring_byte_getlen(bytes)<MSG_LEN_MIN)
		return 0;

	//locate the header
	buffer_ring_byte_read(bytes, pre_bytes, MSG_LEN_HEADER_GW);
	while(memcmp(pre_bytes, MSG_HEADER_GW, MSG_LEN_HEADER_GW)) {
		buffer_ring_byte_get(bytes, pre_bytes, 1); //remove one byte
		if(buffer_ring_byte_getlen(bytes)<MSG_LEN_MIN)
			return 0; 
		buffer_ring_byte_read(bytes, pre_bytes, MSG_LEN_HEADER_GW);
	}
	printf("msg header matched\n");

	if(buffer_ring_byte_getlen(bytes) < MSG_LEN_MIN) {
		return 0;  //header found, but length too short for a message
	}
	else {
		buffer_ring_byte_read(bytes, pre_bytes, MSG_LEN_FIXED); //read the fixed length 

		/*modified code */
		//memcpy(&data_len, pre_bytes+MSG_POS_DATA_LEN, 2);
		//data_len = *(pre_bytes+MSG_POS_DATA_LEN) & 0x00FF;
		//data_len = *(pre_bytes+MSG_POS_DATA_LEN+1) & 0x00FF; //(temporal issues) converted according to server request
		data_len = ((*(pre_bytes+MSG_POS_DATA_LEN) & 0x00FF)<<8) + (*(pre_bytes+MSG_POS_DATA_LEN+1) & 0x00FF);

		//printf("data_len 1st = %d, 2nd = %d\n", *(pre_bytes+MSG_POS_DATA_LEN) &0x00FF, *(pre_bytes+MSG_POS_DATA_LEN+1) &0x00FF);
		//printf("data_len in message=%d buffer length=%d\n",data_len, buffer_ring_byte_getlen(bytes));

		if(buffer_ring_byte_getlen(bytes) < data_len+MSG_LEN_FIXED) { //if buffer is too short for the actual message
			return 0;
		}
		else { //start read the data
			buffer_ring_byte_get(bytes, pre_bytes, MSG_LEN_FIXED);

			data = calloc(data_len, sizeof(char));
			buffer_ring_byte_get(bytes, data, data_len);

			memcpy(&(msg->stamp), pre_bytes+MSG_POS_STAMP, MSG_LEN_STAMP);
			memcpy(&(msg->gateway_id), pre_bytes+MSG_POS_ID_GW, MSG_LEN_ID_GW);
			memcpy(&(msg->dev_id), pre_bytes+MSG_POS_ID_DEV, MSG_LEN_ID_DEV);
			memcpy(&(msg->dev_type), pre_bytes+MSG_POS_DEV_TYPE, MSG_LEN_DEV_TYPE);
			memcpy(&(msg->data_type), pre_bytes+MSG_POS_DATA_TYPE, MSG_LEN_DATA_TYPE);
			msg->data_len = data_len;
			//memcpy(&(msg->data_len), pre_bytes+MSG_POS_DATA_LEN+1, 1);//(temporal issues) only receive the lower 8 bits

			if(msg->data != NULL)
				free(msg->data);
			msg->data = calloc(sizeof(char)*data_len, sizeof(char));
			memcpy(msg->data, data, data_len);
		
			/*	
			printf("body= ");
			for(m = 0; m < data_len; m ++){
				printf("%d " ,(msg->data[m] & 0x00ff));
			}
			printf("\n");
			*/
			free(data);//free the temp data buffer
			return MSG_LEN_FIXED+data_len;
		}
	}
}

int message2bytes(message* msg, char* bytes){
	int len = MSG_LEN_FIXED + msg->data_len;

	//conver the prefix
	memcpy(bytes, MSG_HEADER_GW, MSG_LEN_HEADER_GW);
	memcpy(bytes+MSG_POS_STAMP, &(msg->stamp), MSG_LEN_STAMP);
	memcpy(bytes+MSG_POS_ID_GW, &(msg->gateway_id), MSG_LEN_ID_GW);
	memcpy(bytes+MSG_POS_ID_DEV, &(msg->dev_id), MSG_LEN_ID_DEV);
	memcpy(bytes+MSG_POS_DEV_TYPE, &(msg->dev_type), MSG_LEN_DEV_TYPE);
	memcpy(bytes+MSG_POS_DATA_TYPE, &(msg->data_type), MSG_LEN_DATA_TYPE);

	/*modified code here*/
	//memcpy(bytes+MSG_POS_DATA_LEN, &(msg->data_len), 2);
	//memcpy(bytes+MSG_POS_DATA_LEN+1, &(msg->data_len & 0x00FF), 1);
	*(bytes+MSG_POS_DATA_LEN+1) = msg->data_len & 0x00FF;
	*(bytes+MSG_POS_DATA_LEN) = msg->data_len>>8 & 0x00FF;

	memcpy(bytes+MSG_LEN_FIXED, msg->data, msg->data_len); //conver the data

	return len;
}

/************************************************
  message queue operation
 ************************************************/
message_queue* message_queue_create(){
	message_queue* msg_q = calloc(sizeof(message_queue), sizeof(char));
	msg_q->msg.data = NULL;
	return msg_q;
}

message_queue* message_queue_flush(message_queue* msg_q){
	//move to the head
	msg_q = message_queue_to_head(msg_q);
	message_queue *msg_q_item;
	while(msg_q->next != msg_q){
		msg_q_item = msg_q;
		msg_q = msg_q->next;//move to next msg
		msg_q->prev = msg_q; //set the next msg as the head
		message_flush(&(msg_q_item->msg));
		free(msg_q_item); //free current msg_q
	}
	message_flush(&(msg_q->msg));
	return msg_q;
}

void message_queue_destroy(message_queue* msg_q){
	msg_q = message_queue_flush(msg_q);
	free(msg_q);
}

void message_queue_init(message_queue *msg_q){
	msg_q->prev = msg_q;
	msg_q->next = msg_q;
}

message_queue* message_queue_put(message_queue* msg_q, message* msg){
	if(message_queue_getlen(msg_q)==0){
		message_copy(&(msg_q->msg), msg); //empty queue
	}else{
		msg_q->next = message_queue_create();
		msg_q->next->prev = msg_q;
		msg_q = msg_q->next;
		msg_q->next = msg_q;
		message_copy(&(msg_q->msg), msg);
	}
	return msg_q;
}

message_queue* message_queue_get(message_queue* msg_q, message* msg){
	message_queue *msg_q_item;
	int len = message_queue_getlen(msg_q);
	if(len == 0){
		message_flush(msg);
	}else if(len == 1){
		message_copy(msg, &(msg_q->msg));
		message_flush(&(msg_q->msg));
	}else{
		message_copy(msg,&(msg_q->msg));
		message_flush(&(msg_q->msg));
		msg_q_item = msg_q;
		msg_q = msg_q->next; //move to the next item 
		msg_q->prev = msg_q;
		free(msg_q_item); //delete the current item
	}
	return msg_q;
}

int message_queue_del(message_queue **msg_q){
	message_queue *msg_q_item;
	if((*msg_q)->msg.data == NULL && (*msg_q)->prev == (*msg_q)->next) //if is empty queue
		return 0;
	else{
		message_flush(&((*msg_q)->msg)); //delete message data

		//then delete the message queue item
		if((*msg_q)->prev == *msg_q && (*msg_q)->next == *msg_q) //one message in the queue
			;
		else if((*msg_q)->prev == *msg_q){ //head
			msg_q_item = *msg_q;
			*msg_q = (*msg_q)->next;
			(*msg_q)->prev = *msg_q;
			free(msg_q_item);
		}else if((*msg_q)->next == *msg_q){ //tail
			msg_q_item = *msg_q;
			*msg_q = (*msg_q)->prev;
			(*msg_q)->next = *msg_q;
			free(msg_q_item);
		}else{ //in the middle
			msg_q_item = *msg_q;
			*msg_q = (*msg_q)->next;
			(*msg_q)->prev = msg_q_item->prev;
			msg_q_item->prev->next = *msg_q;
			free(msg_q_item);
		}
		return 1;//return the number of deletion
	}
}

int message_queue_del_stamp(message_queue **msg_q_p, long stamp){
	message_queue *msg_q = *msg_q_p;
	message_queue *msg_q_item;
	msg_q = message_queue_to_head(msg_q);

	int cnt = 0;
	int len = message_queue_getlen(msg_q);

	if(len <=0){ //no msg
		return 0;
	}else if(len==1){ //one msg
		if(msg_q->msg.stamp == stamp){
			cnt++;
			message_flush(&(msg_q->msg));
			memset(&(msg_q->time), 0, sizeof(struct timeval));//reset the time
		}
		return cnt;
	}else{
		while(msg_q->next != msg_q){ //if not the last one
			if(msg_q->msg.stamp == stamp){
				cnt++;
				//message_destroy(msg_q->msg);
				message_flush(&(msg_q->msg));
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
			if(msg_q->msg.stamp == stamp){
				cnt++;
				message_flush(&(msg_q->msg));
				memset(&(msg_q->time), 0, sizeof(struct timeval));//reset the time
			}
		}else{
			if(msg_q->msg.stamp == stamp){
				cnt++;
				message_flush(&(msg_q->msg));
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

int message_queue_find_stamp(message_queue* msg_q, long stamp){ //find if stamp is in the queue, return to number of finding 
	int num = 0;
	if(message_queue_getlen(msg_q) == 0){
		return 0;
	}
	if(message_queue_getlen(msg_q) == 1){
		if(msg_q->msg.stamp == stamp){
			return 1;
		}
		else {
			return 0;
		}
	}
	//for more than 2 msg
	message_queue* msg_q_h = message_queue_to_head(msg_q);
	message_queue* msg_q_t = message_queue_to_tail(msg_q);
	while(msg_q_h != msg_q_t){
		if(msg_q_h->msg.stamp == stamp)
			num ++;
		msg_q_h = msg_q_h->next;
	}
	return num;
}

int message_queue_getlen(message_queue* msg_q){
	msg_q = message_queue_to_head(msg_q);
	int len=0;
	if(msg_q->next == msg_q && msg_q->prev == msg_q){ //one or empty queue
		if(msg_q->msg.data == NULL)
			return 0;
		else
			return 1;
	}
	else{
		len++;
		while(msg_q->next != msg_q){
			len++;
			msg_q=msg_q->next;
		}
		return len;
	}
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

/************************************************
  message creation
 ************************************************/
message* message_create_stat(int stat_len, char* stat, char id_gw[8], char id_dev[8], int dev_type){
	message *msg = message_create();
	msg->data = realloc(msg->data, stat_len);
	memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
	memcpy(msg->dev_id, id_dev, MSG_LEN_ID_DEV);
	msg->data_type = DATA_STAT;
	msg->dev_type = dev_type;
	memcpy(msg->data, stat, stat_len);
	msg->data_len = stat_len;
	return msg;
}

message* message_create_ctrl(int ctrl_len, char* ctrl, char id_gw[8], char id_dev[8], int dev_type){
	message *msg = message_create();
	msg->data = realloc(msg->data, ctrl_len);
	memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
	memcpy(msg->dev_id, id_dev, MSG_LEN_ID_DEV);
	msg->data_type = DATA_CTRL;
	msg->dev_type = dev_type;
	memcpy(msg->data, ctrl, ctrl_len);
	msg->data_len = ctrl_len;
	return msg;
}

message* message_create_sync(int stat_len, char* stat, long u_stamp, char id_gw[8], char id_dev[8], int dev_type){
	message *msg = message_create();
	msg->data = calloc(sizeof(char)*(stat_len+4), sizeof(char));
	memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
	memcpy(msg->dev_id, id_dev, MSG_LEN_ID_DEV);
	msg->data_type = DATA_REQ_SYNC;
	msg->dev_type = dev_type;
	memcpy((void*) msg->data, (void*) &u_stamp, 4);
	memcpy(msg->data+4, stat, stat_len);
	msg->data_len = stat_len+4;
	return msg;
}


message* message_create_stat_update(int stat_len, char* stat, long u_stamp, char id_gw[8], char id_dev[8], int dev_type){
	message *msg = message_create();
	msg->data = calloc(sizeof(char)*(stat_len+4), sizeof(char));
	memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
	memcpy(msg->dev_id, id_dev, MSG_LEN_ID_DEV);
	msg->data_type = DATA_STAT_UPDATE;
	msg->dev_type = dev_type;
	memcpy((void*) msg->data, (void*) &u_stamp, 4);
	memcpy(msg->data+4, stat, stat_len);
	msg->data_len = stat_len+4;
	return msg;
}

message* message_create_req_auth_gw(int lic_len, char* lic, char id_gw[8], long stamp){
	message *msg = message_create();
	msg->data = realloc(msg->data, lic_len);
	memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
	memcpy(msg->dev_id, NULL_DEV, MSG_LEN_ID_DEV);
	msg->data_type = DATA_REQ_AUTH_GW;
	memcpy(msg->data, lic, lic_len);
	msg->data_len = lic_len;
	msg->stamp = stamp;
	return msg;
}

message* message_create_req_auth_dev(char id_gw[8], char id_dev[8], long stamp){
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

message* message_create_pulse(char id_gw[8]){
	message *msg = message_create();
	msg->data = realloc(msg->data, 1);
	memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
	memcpy(msg->dev_id, NULL_DEV, MSG_LEN_ID_DEV);
	msg->data_type = DATA_REQ_PULSE;
	msg->data[0] = TCP_PULSE;
	memset(msg->data, 0, 1);
	msg->data_len = 1;
	return msg;
}

message* message_create_req_stamp(char id_gw[8], long stamp){
	message *msg = message_create();
	msg->data = realloc(msg->data, 1);
	memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
	memcpy(msg->dev_id, NULL_DEV, MSG_LEN_ID_DEV);
	msg->data_type = DATA_REQ_STAMP;
	memset(msg->data, 0, 1);
	msg->data_len = 1;
	msg->stamp = stamp;
	return msg;
}

message* message_create_req_user(char id_gw[8], char id_user[8], long stamp){
	message *msg = message_create();
	msg->data = realloc(msg->data, 1);
	msg->data_len = 1;
	return msg;
}

message* message_create_null(char id_gw[8], long stamp){
	message *msg = message_create();
	msg->data = realloc(msg->data, 1);
	memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
	memcpy(msg->dev_id, NULL_DEV, MSG_LEN_ID_DEV);
	msg->data_type = DATA_NULL;
	memset(msg->data, 0, 1);
	msg->data_len = 1;
	msg->stamp = stamp;
	return msg;
}

message* message_create_ack_auth_local(char id_gw[8], char id_dev[8], int dev_type, char auth_result){
	message *msg = message_create();
	msg->data = realloc(msg->data, 1);
	msg->data[0] = auth_result;
	msg->data_len = 1;
	memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
	memcpy(msg->dev_id, id_dev, MSG_LEN_ID_DEV);
	msg->data_type = DATA_ACK_AUTH_LOCAL;
	return msg;
}

message* message_create_install(char id_gw[8], char id_dev[8], int type) {
	message *msg = message_create();
	msg->data = realloc(msg->data, 1);
	msg->data[0] = 0;
	msg->data_len = 1;
	memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
	memcpy(msg->dev_id, id_dev, MSG_LEN_ID_DEV);
	msg->dev_type = type;
	msg->data_type = DATA_INSTALL;
	return msg;
}

message* message_create_del_install(char id_gw[8], char id_dev[8], int result){
	message *msg = message_create();
	msg->data = realloc(msg->data, 1);
	if (result >= 0)
		msg->data[0] = 0x00;
	else 
		msg->data[0] = 0xff;

	msg->data_len = 1;
	memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
	memcpy(msg->dev_id, id_dev, MSG_LEN_ID_DEV);
	msg->dev_type = 1;
	msg->data_type = DATA_DEL_INSTALL;
	return msg;
}

/*
message* message_create_install_info(char id_gw[8], char id_dev[8], int dev_type, int len_descrip, char* descrip){
	message *msg = message_create();
	msg->data_len = 2+len_descrip;
	msg->data = realloc(msg->data, sizeof(char)*(2+len_descrip));
	memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
	memcpy(msg->dev_id, id_dev, MSG_LEN_ID_DEV);
	msg->dev_type = dev_type;
	msg->data_type = DATA_INSTALL_INFO;
	return msg;
}
*/

message* message_create_set_password_ack(char id_gw[8]){
	message *msg = message_create();
	memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
	memcpy(msg->dev_id, NULL_DEV, MSG_LEN_ID_DEV);
	msg->dev_type = 0;
	msg->data = realloc(msg->data, sizeof(char));
	msg->data[0] = AUTH_OK;
	msg->data_type = DATA_SET_PASSWORD_ACK;
	msg->data_len = 1;
	return msg;
}

message* message_create_set_lic_ack(char id_gw[8]){
	message *msg = message_create();
	memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
	memcpy(msg->dev_id, NULL_DEV, MSG_LEN_ID_DEV);
	msg->dev_type = 0;
	msg->data = realloc(msg->data, 1);
	msg->data[0] = AUTH_OK;
	msg->data_type = DATA_SET_LIC_ACK;
	msg->data_len = 1;
	return msg;
}

message* message_create_del_znode(char id_gw[8], char id_dev[8]){
	message *msg = message_create();
	memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
	memcpy(msg->dev_id, id_dev, MSG_LEN_ID_DEV);
	msg->dev_type = 0;
	msg->data = realloc(msg->data, 1);
	msg->data[0] = 0;
	msg->data_type = DATA_DEL_ZNODE;
	msg->data_len = 1;
	return msg;
}

message* message_create_ack_install_op(char id_gw[8], char id_dev[8], int op_code, int result){
	message *msg = message_create();
	msg->data = realloc(msg->data, 13);

	memcpy(msg->data, id_dev, sizeof(int));
	memcpy(msg->data+8, &(op_code), sizeof(int));

	if (result >= 0)
		msg->data[12] = 0x00;
	else 
		msg->data[12] = 0xff;

	msg->data_len = 13;
	memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
	memcpy(msg->dev_id, id_dev, MSG_LEN_ID_DEV);
	msg->dev_type = 0;
	msg->data_type = DATA_ACK_INSTALL_OP;
	return msg;
}

message* message_create_ack_scene_op(char id_gw[8], char id_major[8], char id_minor[8], int op_code, int result){
	message *msg = message_create();
	msg->data = realloc(msg->data, 21);
	memcpy(msg->data, id_major, 8*sizeof(char));
	memcpy(msg->data+8, id_minor, 8*sizeof(char));
	memcpy(msg->data+16, &(op_code), sizeof(int));

	if (result >= 0)
		msg->data[20] = 0x00;
	else 
		msg->data[20] = 0xff;

	msg->data_len = 21;

	memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
	memcpy(msg->dev_id, NULL_DEV, MSG_LEN_ID_DEV);
	msg->dev_type = 0;
	msg->data_type = DATA_ACK_SCENE_OP;
	return msg;
}


message* message_create_ack_server_conn(char id_gw[8], char server_conn){
	message *msg = message_create();
	msg->data = realloc(msg->data,1);
	msg->data[0] = server_conn;
	memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
	msg->dev_type = 0;
	msg->data_type = DATA_ACK_SERVER_CONN;

	return msg;
};
