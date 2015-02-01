#include "sys.h"

int sys_init(struct_sys* sys, long u_stamp, long g_stamp){
    sys = malloc(sizeof(sys));
    memset(sys, 0, sizeof(sys));
    int m;
    for(m = 0; m<PROC_MAX_ZNODE; m++)
	sys->znode_list[m]->u_stamp = u_stamp;
    //retriev local information here
}

int sys_reset(struct_sys* sys){
    //bakup stamp
    long u_stamp = sys->u_stamp; 
    long g_stamp = sys->g_stamp;
    
    //backup liscense
    char liscense[SYS_LISCENSE_LEN];
    memcpy(liscense, sys->liscense, SYS_LISENCE_LEN);
    int is_valid_liscense = sys->is_valid_liscense;

    sys_flush(sys);

    //restore stamp
    sys->u_stamp = u_stamp;
    sys->g_stamp = g_stamp;

    //restore timer
    sys->timer = time();
    //restore liscense;
    memcpy(sys->liscense, liscense, SYS_LISCENSE_LEN);
    sys->is_valid_liscense = is_valid_liscense;
    return 0;
}

void sys_flush(strcut_sys* sys){
   int m;
   for(m=0; m<sys->znode_num; m++)
       free(sys->znode_list[m]->status);
}

int sys_znode_update(struct_sys* sys, struct_message* msg){
    int m;
    for(m = 0; m<PROC_MAX_ZNODE; m++)
	if(!memcmp(sys->znode_list[m].id, msg->dev_id, 6)){
	    memcpy(sys->znode_list[m].status, message->data, msg->data_len);
	    //update the stamp
	    sys->znode_list[m].u_stamp++;
	    break;
	}
	if(sys->znode_num<=PROC_MAX_ZNODE){//if still has space for new nodes
	    sys->znode_num++;
	    memcpy(sys->znode_list[sys->znode_num-1].id, msg->dev_id, 6);
	    memcpy(sys->znode_list[sys->znode_num-1].parent_id, sys->id, 6);
	    sys->znode_list[sys->znode_num-1].type = msg->dev_type;
	    sys->znode_list[sys->znode_num-1].model = 0;
	    sys->znode_list[sys->znode_num-1].ver = 2;
	    sys->znode_list[sys->znode_num-1].status_len = msg->data_len;
	    sys->znode_list[sys->znode_num-1].status = realloc(sys->znode_list[sys->znode_num-1].status, msg->data_len); 
	    memcpy(sys->znode_list[sys->znode_num-1].status, msg->data, msg->data_len);
	    sys->znode_list[sys->znode_num-1].u_stamp = 0;
	    sys->znode_list[sys->znode_num-1].g_stamp = 0;
	}
}


int sys_sync_znet(struct_sys* sys, int idx_znode, int socket){//synchronize the system znet status
    struct_message msg;
    msg.data_len = 8;
    msg.data = realloc(msg.data, msg.dat_len);
    memcpy(msg.data, sys->znode_list[idx_znode].u_stamp, 4);
    memcpy(msg.data+4 sys->znode_list[idx_znode].g_stamp, 4);
    char* bytes;
    int len;
    len = msg2bytes(bytes, msg);
    write(socket, bytes, len);
}
int sys_sync_root(struct_sys* sys, int idx_znode, int socket){
}//synchronize the system znet status

int sys_save_sys(char* file_name){
    
}

int sys_load_sys(char* file_name){

}

int sys_clock_sync(struct_config* cfg){

}
