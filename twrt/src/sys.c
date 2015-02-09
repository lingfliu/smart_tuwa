#include "sys.h"

//initialize an empty system
int sys_init(struct_sys* sys, struct_config *cfg){
    sys = malloc(sizeof(struct_sys));
    memset(sys, 0, sizeof(struct_sys));
    //get default system variables

    sys_get_id(sys);
    sys_get_lic(sys, cfg->file_lic); 
    sys->timer_net_hb = time();
    sys->timer_reset = time();
}

void sys_get_id(struct_sys* sys){
    sys->id = "000000";
}
void sys_get_lic(struct_sys* sys, char* file_lic){
    memset(sys->lic, 0, SYS_LIC_LEN);
}

int sys_get_auth(struct_sys* sys, struct_inet* client){   
    int result = -1;
    struct_message* msg;
    buffer_ring_byte* buff;
    buff = buffer_ring_byte_create(buff, 500);
    char* bytes;
    msg = struct_message_create_sys(struct_sys* sys, msg,  MSG_SYS_AUTH_REQ);
    int len = msg2bytes(msg, &bytes);   

    //send auth req
    write(client->fd, bytes, len);
    free(msg);
    free(bytes);

    //after sending the req, wait to get ack
    bytes = realloc(bytes, 255);

    //this loop will break only when: server disconnected / ack negative
    while(1){
	if(bytes2msg(buff,msg)<0){
	    len = read(client->fd, bytes, 255);
	    if(len <= 0)
		break;
	    buffer_ring_byte_put(buff, bytes, len);
	}else{
	    if(msg->type == DATA_TYPE_AUTH_ACK){
		if(msg->data_len == SYS_COOKIE_LEN)
		    result = 0;
		break;
	    }
	}
    }

    if(result<0)
	sys->is_valid_lic = 0;
    else{
	sys->is_valid_lic = 1;
	memcpy(sys->cookie, msg->data, SYS_COOKIE_LEN);
    }
    free(bytes);
    free(buff);
    return result;
}

long sys_get_stamp(struct_sys* sys, struct_inet* client){
    long stamp = -1;
    if(!sys->is_valid_lic)
	return stamp;

    struct_message* msg;
    buffer_ring_byte* buff;
    buff = buffer_ring_byte_create(buff, 500);
    char* bytes;
    msg = struct_message_create_sys(sys, msg,  MSG_SYS_STAMP_REQ);
    int len = msg2bytes(msg, &bytes);   

    //send auth req
    write(client->fd, bytes, len);
    free(msg);
    free(bytes);

    //after sending the req, wait to get ack
    bytes = realloc(bytes, 255);

    //this loop will break only when: server disconnected
    while(1){
	if(bytes2msg(buff,msg)<0){
	    len = read(client->fd, bytes, 255);
	    if(len <= 0)
		break;
	    buffer_ring_byte_put(buff, bytes, len);
	}else{
	    if(msg->type == DATA_TYPE_STAMP_ACK){
		if(msg->data_len == 4)
		    memcpy(&stamp, msg->data, 4);
		break;		
	    }
	}
    }

    return stamp;
}

void sys_flush(struct_sys* sys){
    memset(sys->status, 0, SYS_STATUS_LEN);//flush the system status

    int m;
    for(m = 0; m < sys->znode_num; m++)
	znode_flush(&(sys->znode[m]i)); //flush the znet status

    sys->timer_net_hb = time();
    sys->timer_reset = time();
}

int sys_znode_update(struct_sys* sys, struct_message* msg){
    int idx = -1;
    int m;
    for(m = 0; m<PROC_MAX_ZNODE; m++)
	if(!memcmp(sys->znode_list[m].id, msg->dev_id, 6)){
	    memcpy(sys->znode_list[m].status, message->data, msg->data_len);
	    //update the stamp
	    sys->znode_list[m].u_stamp++;
	    idx = m;
	    break;
	}
	if(sys->znode_num<=PROC_MAX_ZNODE){//if still has space for new nodes
	    sys->znode_num++;
	    idx = sys->znode_num-1;
	    memcpy(sys->znode_list[idx].id, msg->dev_id, 6);
	    memcpy(sys->znode_list[idx].parent_id, sys->id, 6);
	    sys->znode_list[idx].type = msg->dev_type;
	    sys->znode_list[idx].model = 0;
	    sys->znode_list[idx].ver = 2;
	    sys->znode_list[idx].status_len = msg->data_len;
	    sys->znode_list[idx].status = realloc(sys->znode_list[sys->znode_num-1].status, msg->data_len); 
	    memcpy(sys->znode_list[idx].status, msg->data, msg->data_len);
	    sys->znode_list[idx].u_stamp = sys->u_stamp; //set stamp as no less than the root stamp
	    sys->znode_list[idx].g_stamp = sys->g_stamp;
	}
	return idx;
}


void sys_sync_znode(struct_sys* sys, int idx_znode, struct_message_queue** msg_q_tx){//synchronize the system znet status
    struct_message *msg;
    msg = message_create_sync_znode(&(sys->znode[idx_znode]), msg);
    message_queue_put(msg_q_tx, msg);
    free(msg);
}

void sys_sync_root(struct_sys* sys, struct_message_queue** msg_q_tx){
    struct_message *msg;
    msg = message_cretae_sys_root(sys, msg);
    message_queue_put(msg_q_tx, msg);
    free(msg);
}

void sys_net_hb(struct_sys* sys, truct_message_queue** msg_q_tx){
    struct_message *msg;
    msg = message_create_net(sys, MSG_NET_HB, msg);
    message_queue_put(msg_q_tx, msg);
    free(msg);
}

//message creation
struct_message* message_create_empty(struct_message* msg){
    msg = realloc(sizeof(struct_message));
    memset(msg, 0, sizeof(struct_message));
    return msg;
}

struct_message* message_create_sys(struct_message* msg, struct_sys* sys, int type){

    msg = realloc(sizeof(struct_message));
    memset(msg, 0, sizeof(struct_message));
    memcpy(msg->gateway_id, sys->id, 6);

    switch(type){
	case MSG_SYS_NET_HB:
	    msg->data_type = DATA_TYPE_NET_HB;
	    msg->data_len = 1;
	    msg->data = realloc(msg->data, 1);
	    memcpy(msg->data,DATA_NET_HB, 1);  
	    msg->stamp = ++sys->msg_tx_stamp;
	    break;
	case MSG_SYS_AUTH_REQ:
	    msg->data_type = DATA_TYPE_AUTH_REQ;
	    msg->data_len = SYS_LIC_LEN;
	    msg->data = realloc(msg->data, SYS_LIC_LEN);
	    memcpy(msg->data, sys->lic, SYS_LIC_LEN);
	    msg->stamp = ++sys->msg_tx_stamp;
	    break;
	case MSG_SYS_STAMP_REQ:
	    msg->data_type = DATA_TYPE_STAMP_REQ;
	    msg->data_len = 4;
	    msg->data = realloc(msg->data, 4);
	    memset(msg->data, 0, 4);
	    msg->stamp = ++sys->msg_tx_stamp;
	    break;
	default:
	    break;
    }
    return msg;
}

struct_message* message_create_sync_znode(struct_znode* znode, struct_message* msg){
    msg = realloc(sizeof(struct_message));
    memset(msg, 0, sizeof(struct_message));
    memcpy(msg->gateway_id, sys->id, 6);
    memcpy(msg->dev_id, znode->id, 6);
    msg->dev_type = znode->type;
    msg->data_type = DATA_TYPE_SYNC;
    msg->data_len = znode->status_len+4;
    msg->data = realloc(msg->data, msg->data_len);
    memcpy(msg->data, znode->status, msg->data_len-4);
    memcpy(msg->data+msg->data_len-4, znode->u_stamp, 4);

    return msg;
}
struct_message* message_create_sync_root(struct_sys* sys, struct_message* msg){
    msg = realloc(sizeof(struct_message));
    memset(msg, 0, sizeof(struct_message));
    memcpy(msg->gateway_id, sys->id, 6);
    msg->data_type = DATA_TYPE_SYNC;
    msg->data_len = SYS_STATUS_LEN+4;
    msg->data = realloc(msg->data, msg->data_len);
    memcpy(msg->data, znode->status, msg->data_len-4);
    memcpy(msg->data+msg->data_len-4, znode->u_stamp, 4);

    return msg;
}

