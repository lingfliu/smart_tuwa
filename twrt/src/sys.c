#include "sys.h"

//znode operations
/////////////////////////////////////////////////////
int znode_isempty(znode *znd){
    if(znd->status_len == 0 && znd->status == NULL && !memcmp(znd->id, NULL_DEV, MSG_LEN_ID_DEV)){
		return 1;
	}
    else {
		return 0;
	}
}

int znode_copy(znode* znode_dst, znode* znode_src){
    if(znode_isempty(znode_src)) {
		return -1;
	}
	memcpy(znode_dst, znode_src, sizeof(znode));
    znode_dst->status = realloc(znode_dst->status, znode_dst->status_len);
    memcpy(znode_dst->status, znode_src->status, znode_dst->status_len);
    return 0;
}

void znode_create(znode* znd, message *msg){
    memcpy(znd->id, msg->dev_id, MSG_LEN_ID_DEV);
    znd->type = msg->dev_type;
    znd->model = DEFAULT_MODEL;
    znd->ver = DEFAULT_VER;
    znd->status_len = msg->data_len;
    znd->status = realloc(znd->status, msg->data_len);
	//printf("status_len = %d", msg->data_len);
    memcpy(znd->status, msg->data, msg->data_len);
    znd->net_status = ZNET_ON;
    znd->u_stamp = 0;
}

void znode_update(znode* znd, message *msg){
    memcpy(znd->status, msg->data, msg->data_len);
    znd->net_status = ZNET_ON;
    znd->u_stamp++;
}

void znode_flush(znode* znd){
    memset(znd->status, 0, znd->status_len);
    free(znd->status);
    memset(znd, 0, sizeof(znode));
}

//sys_t operation
//////////////////////////////////////////////////////
void sys_get_id(sys_t* sys, char* id_file){
    FILE* fp;
    fp = fopen(id_file, "r");
    if(fp == NULL){
		perror("Cannot read id file, abort.");
    }
    if(fread(sys->id, sizeof(char), MSG_LEN_ID_GW, fp)!= MSG_LEN_ID_GW){
		perror("read error");
	}
    fclose(fp);
}

void sys_get_lic(sys_t* sys, char* lic_file){
    FILE *fp;
    fp = fopen(lic_file, "r");
    if(fp == NULL){
		perror("Cannot read id file, abort.");
    }
    if(fread(sys->lic, sizeof(char), SYS_LEN_LIC, fp) != SYS_LEN_LIC){
		perror("lic read error");
	}
    fclose(fp);
}

//initialize an empty sys_t
void sys_init(sys_t* sys){
	int m;
    memset(sys, 0, sizeof(sys_t));

    sys_get_id(sys, FILE_ID_SYS);
    sys_get_lic(sys, FILE_LIC); 
    sys->lic_status= LIC_UNKNOWN;
    sys->server_status = SERVER_DISCONNECT;
	sys->u_stamp = -1; //status stamp
	sys->tx_msg_stamp = 0;

	for(m = 0; m < PROC_ZNODE_MAX; m++){
		memcpy(sys->znode_list[m].id, NULL_DEV, MSG_LEN_ID_DEV);
		sys->znode_list[m].u_stamp = -1;
	}	
}


int sys_znode_update(sys_t* sys, message* msg){
    int idx = -1;
    int idx_empty = -1;
    int m;
	for(m = 0; m < PROC_ZNODE_MAX; m++){
		if(!memcmp(msg->dev_id, sys->znode_list[m].id, MSG_LEN_ID_DEV)){ //if id_dev equal
			idx = m;
			break;
		}
	}

	if( idx < 0 ) {//not in the list 
		for(m = 0; m < PROC_ZNODE_MAX; m++ ) {
			if(znode_isempty( &(sys->znode_list[m]) ) ){
				idx_empty = m;
				break;
			}
		}

		if( idx_empty < 0 ) { //znode list is full, do nothing
			return -1; 
		}
		else { //add new node in the list
			znode_create(&(sys->znode_list[idx_empty]), msg);
			sys->znode_list[idx_empty].u_stamp = sys->u_stamp; //set u_stamp as system u_stamp
			return idx_empty;
		}
	}
	else { //already in the list, update
		znode_update(&(sys->znode_list[idx]), msg); //u_stamp will be updated here
		return idx;
	}
}

int sys_get_znode_idx(sys_t *sys, char id[8]){
    int m;
    int idx = -1;
    for(m = 0; m < PROC_ZNODE_MAX; m++){
	if(!memcmp(sys->znode_list[m].id, id, MSG_LEN_ID_DEV)){
	    idx = m;
	    break;
	}else{
	    continue;
	}
    }
    return idx;
}

int sys_get_znode_num(sys_t *sys){
    int num = 0;
    int m;
    for(m = 0; m < PROC_ZNODE_MAX; m++){
	if(!znode_isempty(&(sys->znode_list[m])))
	    num++;
    }
    return num;
}

message* sys_sync(sys_t *sys, message *msg){
   int stamp; 
   memcpy(&stamp, msg->data, 4);
   int idx;

   if(!memcmp(&(msg->dev_id), NULL_DEV, 8)){ //root sync
	   if(sys->u_stamp < stamp){ //if msg contains newer status, update
		   memcpy(sys->status, msg->data+4, SYS_LEN_STATUS);
		   sys->u_stamp = stamp; //don't forget to update the stamp
		   return NULL;
	   }else if(sys->u_stamp == stamp){
		   return NULL;
	   }else{ //if local is newer, send back a sync msg
		   return message_create_sync(SYS_LEN_STATUS, sys->status, sys->u_stamp, sys->id, NULL_DEV, 0, 0);
	   }
   }

   idx = sys_get_znode_idx(sys, msg->dev_id);
   if(idx<0) {
	   return NULL;
   }
   if(sys->znode_list[idx].u_stamp < stamp) { //if msg contains newer status, update
	   memcpy(sys->znode_list[idx].status, msg->data+4, msg->data_len-4);
	   sys->znode_list[idx].u_stamp = stamp; //don't forget to update the stamp
	   return NULL;
   }
   else { //if local status is newer, send back a sync msg
	   return message_create_sync(sys->znode_list[idx].status_len, sys->znode_list[idx].status, sys->znode_list[idx].u_stamp, sys->id, sys->znode_list[idx].id, sys->znode_list[idx].type, 0);
   }
}

void sys_save(sys_t *sys, char* save_file){
    int m;
    int fp;
    fp = open(save_file, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
    
    if(fp == -1)
	perror("save file open error");
	//save the file as 
	if(write(fp, &(sys->status), sizeof(sys->status)) != sizeof(sys->status)){
		perror("write error");
	}
	if(write(fp, &(sys->u_stamp), sizeof(sys->u_stamp)) != sizeof(sys->u_stamp)){
		perror("write error");
	}
	if(write(fp, sys->cookie, sizeof(sys->cookie)) != sizeof(sys->cookie)){
		perror("write error");
	}
	for(m = 0; m<PROC_ZNODE_MAX; m++){
		if(write(fp, sys->znode_list[m].id, MSG_LEN_ID_DEV) != MSG_LEN_ID_DEV){
			perror("write error");
		}
		if(write(fp, &(sys->znode_list[m].type), sizeof(int)) != sizeof(int)){
			perror("write error");
		}
		if(write(fp, &(sys->znode_list[m].u_stamp), sizeof(long)) != sizeof(long)){
			perror("write error");
		}
	}

}

void sys_load(sys_t *sys, char* save_file){
	int m;
	int fp;
	fp = open(save_file, O_RDONLY);

	if(fp == -1)
		perror("save file open error");

	//save the file as 
	if(read(fp, sys->status, SYS_LEN_STATUS) != SYS_LEN_STATUS){
		perror("read error");
	}
	if(read(fp, &(sys->u_stamp), sizeof(long)) != sizeof(long)){
		perror("read error");
	}
	if(read(fp, sys->cookie, SYS_LEN_COOKIE) != SYS_LEN_COOKIE){
		perror("read error");
	}

	for(m = 0; m<PROC_ZNODE_MAX; m++){
		if(read(fp, sys->znode_list[m].id, MSG_LEN_ID_DEV) != MSG_LEN_ID_DEV){
			perror("read error");
		}
		if(read(fp, &(sys->znode_list[m].type), sizeof(int)) != sizeof(int)){
			perror("read error");
		}
		if(read(fp, &(sys->znode_list[m].u_stamp), sizeof(long)) != sizeof(long)){
			perror("read error");
		}
	}
}
