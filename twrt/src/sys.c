#include "sys.h"

/**znode operation************************************
******************************************************/
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
    memcpy(znd->status, msg->data, msg->data_len);
    znd->znet_status = ZNET_ON;
    znd->u_stamp = 0;
}

void znode_update(znode* znd, message *msg){
    memcpy(znd->status, msg->data, msg->data_len);
    znd->znet_status = ZNET_ON;
    znd->u_stamp++;
}

//this function only remove the status of the znode
void znode_flush(znode* znd){
    memset(znd->status, 0, znd->status_len);
}

//this function remove all infos of the znode
void znode_delete(znode* znd){
	free(znd->status);
    memset(znd, 0, sizeof(znode));
}

int sys_znode_update(sys_t* sys, message* msg){
    int idx = -1;
    int idx_empty = -1;
    int m;
	for(m = 0; m < ZNET_SIZE; m++){
		if(!memcmp(msg->dev_id, sys->znode_list[m].id, MSG_LEN_ID_DEV)){ //if id_dev equal
			idx = m;
			break;
		}
	}

	if( idx < 0 ) {//not in the list 
		for(m = 0; m < ZNET_SIZE; m++ ) {
			if(znode_isempty( &(sys->znode_list[m]) ) ){
				idx_empty = m;
				break;
			}
		}
		if( idx_empty == -1 ) { //znode list is full, do nothing
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
    for(m = 0; m < ZNET_SIZE; m++) {
		if(!memcmp(sys->znode_list[m].id, id, MSG_LEN_ID_DEV)) {
			idx = m;
			break;
		}
		else {
			continue;
		}
    }
    return idx;
}

int sys_get_znode_num(sys_t *sys) {
    int num = 0;
    int m;
    for(m = 0; m < ZNET_SIZE; m++) {
	if(!znode_isempty(&(sys->znode_list[m])))
	    num++;
    }
    return num;
}

/**localuser operation*********************************
******************************************************/
int localuser_isnull(localuser *usr){
    //if(!(usr->id, NULL_USER, 8) || usr->skt < 0){
    if(usr->skt < 0){
		return 1;
	}
	else {
		return 0;
	}
}

int localuser_create(localuser *usr, char id[8], int skt){
	//the thrd_rx and thrd_tx will not be activated here
	//this function binds skt and user
	memcpy(usr->id, id, 8*sizeof(char));
	usr->skt = skt;

	//initialize the timer
	gettimeofday(&(usr->time_lastactive), NULL);

	usr->is_authed = AUTH_OK;
	return 0;
}

void localuser_delete(localuser *usr){
	memcpy(usr->id, NULL_USER, 8*sizeof(char));
	close(usr->skt);
	usr->skt = -1;
	usr->is_authed = AUTH_NO;
	buffer_ring_byte_flush(&(usr->buff));
}

localuser* sys_localuser_login(sys_t* sys, int skt){
	int m;
	int idx = -1;
	localuser *usr;
	for (m = 0; m < LOCALUSER_SIZE; m ++) {
		if ( localuser_isnull( &(sys->localuser_list[m])) ) {
			sys->localuser_list[m].skt = skt;
			idx = m;
			break;
		}
		else {
			continue;
		}
	}

	//if list is full, do the evict
	if (idx == -1){
		sys_localuser_evict(sys);
		usr = sys_localuser_login(sys, skt); //and login again
		return usr;
	}
	else {
		/***********************
		   debug codec
		 ***********************/
		//printf("new user index = %d\n",idx);
		sys->localuser_list[idx].idx = idx;

		return &(sys->localuser_list[idx]);
	}

} 

//localuser logout (active logout)
int sys_localuser_logout(sys_t* sys, char id[8]){
	int idx = sys_get_localuser_idx(sys, id);
	if (idx < 0) {
		return -1;
	}
	else {
		//close the socket and delete the user
		localuser_delete( &(sys->localuser_list[idx]) );
		return 0;
	}
}

//force logout (passive logout) a localuser if: 1. localuser_list is full, 2. socket do not receive any data after timeout
int sys_localuser_evict(sys_t* sys) {
	int m;
	int cnt;
	long max_timediff;
	long timedi;
	int idx;
	struct timeval timer;

	gettimeofday(&timer, NULL);

	cnt = 0;
	for (m = 0; m < LOCALUSER_SIZE; m ++) {
		if (timediff_s(sys->localuser_list[m].time_lastactive, timer) > DEFAULT_LOCALHOST_TIMEOUT) {
			localuser_delete( &(sys->localuser_list[m]) );
			cnt ++;
		}
	}

	//if no user is timeouted, remove the oldest user
	if (cnt == 0) {
		idx = 0;
		max_timediff = timediff_s(sys->localuser_list[0].time_lastactive, timer);
		for (m = 1; m < LOCALUSER_SIZE; m ++) {
			timedi = timediff_s(sys->localuser_list[m].time_lastactive, timer);
			if ( timedi > max_timediff) {
				max_timediff = timedi;
				idx = m;
			}
		}
		localuser_delete(&(sys->localuser_list[idx]));
		/***********************
		  debug codec
		 ***********************/
		printf("too many sockets, removed localuser %d\n",idx);

		cnt ++;
	}
	return cnt;
}

int sys_get_localuser_idx(sys_t* sys, char id[8]){
	int idx = -1;
	int m;
	for(m = 0; m < LOCALUSER_SIZE; m ++) {
		if(!memcmp(sys->localuser_list[m].id, id, 8)){
			idx = m;
			break;
		}
	}
	return idx;
}

int sys_get_localuser_num(sys_t* sys){
	int cnt = 0;
	int m;
	for(m = 0; m < LOCALUSER_SIZE; m ++) {
		if(!localuser_isnull( &(sys->localuser_list[m])) ) {
			cnt ++;
		}
	}
	return cnt;
}

/************************************************
  sys_t operation 
 ************************************************/
void sys_get_id(sys_t* sys, char* id_file){
    FILE* fp;
    fp = fopen(id_file, "r");
    if(fp == NULL){
		//perror("Cannot read id file, abort.");
    }
    if(fread(sys->id, sizeof(char), MSG_LEN_ID_GW, fp)!= MSG_LEN_ID_GW){
		//perror("read error");
	}
    fclose(fp);
}

void sys_get_lic(sys_t* sys, char* lic_file){
    FILE *fp;
    fp = fopen(lic_file, "r");
    if(fp == NULL){
		//perror("Cannot read id file, abort.");
    }
    if(fread(sys->lic, sizeof(char), SYS_LEN_LIC, fp) != SYS_LEN_LIC){
		//perror("lic read error");
	}
    fclose(fp);
}

//initialize an empty sys_t
void sys_init(sys_t* sys){
	int m;
    memset(sys, 0, sizeof(sys_t));

	//get id, authcode, and lic
    sys_get_id(sys, FILE_ID_SYS);
    sys_get_lic(sys, FILE_LIC); 

	//reset status
    sys->lic_status= LIC_UNKNOWN;
    sys->server_status = SERVER_DISCONNECT;
	sys->serial_status = SERIAL_OFF;
	sys->local_status = LOCALHOST_CLOSED;

	//initialize status stamp
	sys->u_stamp = -1; 

	//reset counters
	sys->tx_msg_stamp = 0;

	//initialize znet
	for(m = 0; m < ZNET_SIZE; m++){
		memcpy(sys->znode_list[m].id, NULL_DEV, MSG_LEN_ID_DEV);
		sys->znode_list[m].u_stamp = -1;
	}	

	//initialize localuser
	for(m = 0; m < LOCALUSER_SIZE; m++){
		memcpy(sys->localuser_list[m].id, NULL_USER, 8);
		buffer_ring_byte_create( &(sys->localuser_list[m].buff), BUFF_RING_LEN );
		sys->localuser_list[m].skt = -1;
		sys->localuser_list[m].is_authed = 0;
	}
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
		   return message_create_sync(SYS_LEN_STATUS, sys->status, sys->u_stamp, sys->id, NULL_DEV, 0);
	   }
   }
   else {
	   idx = sys_get_znode_idx(sys, msg->dev_id);
	   if(idx<0) {
		   return NULL;
	   }
	   else if(sys->znode_list[idx].u_stamp <= stamp) { //if msg contains newer status, update
		   memcpy(sys->znode_list[idx].status, msg->data+4, msg->data_len-4);
		   sys->znode_list[idx].u_stamp = stamp; //don't forget to update the stamp
		   return NULL;
	   }
	   else { //if local status is newer, send back a sync msg
		   return message_create_sync(sys->znode_list[idx].status_len, sys->znode_list[idx].status, sys->znode_list[idx].u_stamp, sys->id, sys->znode_list[idx].id, sys->znode_list[idx].type);
	   }
   }
}

