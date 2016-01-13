#include "sys.h"

/**znode operation************************************
******************************************************/
int znode_isempty(znode *znd){
    if(znd->status_len == 0 || znd->type == 0){
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
		/*deprecated in earlier version*/
		
		/*restore earlier version */
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
		/******************************************************/
		/*modified: if node not in the list, simply return  -1*/
		/******************************************************/
		//return idx;
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
	usr->is_authed = 0;
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
		sys->localuser_list[idx].is_authed = 0; //user has to be authed after login in order to work

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
	else if(fread(sys->id, sizeof(char), MSG_LEN_ID_GW, fp)!= MSG_LEN_ID_GW){
		fclose(fp);
		//perror("read error");
	}
	else {
		printf("sys id = %s\n", sys->id);
		fclose(fp);
	}
}

void sys_get_lic(sys_t* sys, char* lic_file){
    FILE *fp;
    fp = fopen(lic_file, "r");
    if(fp == NULL){
		//perror("Cannot read id file, abort.");
    }
	else if(fread(sys->lic, sizeof(char), SYS_LEN_LIC, fp) != SYS_LEN_LIC){
		//perror("lic read error");
		fclose(fp);
	}
	else {
		fclose(fp);
	}
}

//initialize an empty sys_t
void sys_init(sys_t* sys){
	int m;

	//char password[8];
    memset(sys, 0, sizeof(sys_t));

	//get id, authcode, and lic
    sys_get_id(sys, FILE_ID_SYS);
    get_lic(sys->lic); 
	printf("sys lic = %s, id = %s\n", sys->lic, sys->id);

	/*
	 * test code, password setting working
	 */
	/*
	set_lic(sys->lic, "1234567812345678");
    get_lic(sys->lic); 
	printf("lic = %s\n", sys->lic);
	get_password(password);
	set_password(password, "11111111");
	get_password(password);
	printf("password = %s\n", password);
	*/

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
	/* removed */
	//sys_get_dev_install(sys, FILE_INSTALL);
	for (m = 0; m < ZNET_SIZE; m ++){
		sys->znode_list[m].type = 0;
		sys->znode_list[m].u_stamp = -1;
	}

	/*
	for (m = 0; m < ZNET_SIZE; m ++){
		if (!znode_isempty(&(sys->znode_list[m]))) {
			printf("id=%s, type=%d, description=%s\n", sys->znode_install_list[m].id, sys->znode_install_list[m].type, sys->znode_install_list[m].descrip);
		}
	}
	*/

	/*
	 * initialize fan
	 */
	sys->fan_status = STAT_OFF;
	//send initial control to fan;
	if (sys->fan_status == STAT_OFF){
		fan_control(1);
	}
	else if (sys->fan_status == STAT_ON){
		fan_control(0);
	}
	else {
		//Do nothing
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

void sys_get_dev_install(sys_t *sys, char* install_file) {
	FILE* fp;
	char str[128];
	char strTmp[128];
	int cnt;
	int m;
	int idx[3];
	int idxCnt;
	int len_descrip;

	fp = fopen(FILE_INSTALL,"r");
	if(fp == NULL){
		printf("file cannot be opened\n");
		//if file not existing, create one
		fp = fopen(FILE_INSTALL, "w+");
		if (fp != NULL){
			fclose(fp);
		}
		return;
	}
	else {
		printf("read file: %s\n", FILE_INSTALL);
		cnt = 0;
		while(fgets(str,128,fp)!=NULL) {
			idxCnt = 0;
			for(m = 0; m < 128; m ++) {
				if (str[m] == ':'){
					idx[idxCnt++] = m;
				}
				if (idxCnt > 3) {
					break;
				}
			}

			//set the znode_install_list

			printf("get znode install \n");
			memcpy(sys->znode_install_list[cnt].id, str, sizeof(char)*idx[0]);

			bzero(strTmp, sizeof(char)*128);
			memcpy(strTmp, str+idx[0]+1, sizeof(idx[1]-idx[0]-1));
			sscanf(strTmp, "%d", &(sys->znode_install_list[cnt].type));

			bzero(sys->znode_install_list[cnt].descrip, sizeof(char)*50);
			bzero(strTmp, sizeof(char)*128);
			memcpy(strTmp, str+idx[1]+1, sizeof(char)*(idx[2]-idx[1]-1));
			len_descrip = strlen(strTmp);
			sys->znode_install_list[cnt].len_descrip = len_descrip;
			memcpy(sys->znode_install_list[cnt].descrip, strTmp, sizeof(char)*len_descrip);

			//set the znode_list
			memcpy(sys->znode_list[cnt].id, sys->znode_install_list[cnt].id, 8);
			sys->znode_list[cnt].type = sys->znode_install_list[cnt].type;
			sys->znode_list[cnt].model = DEFAULT_MODEL;
			sys->znode_list[cnt].ver = DEFAULT_VER;
			sys->znode_list[cnt].status_len = get_znode_status_len(sys->znode_list[cnt].type);
			sys->znode_list[cnt].status = calloc(sys->znode_list[cnt].status_len, sizeof(char));
			sys->znode_list[cnt].znet_status = ZNET_ON;
			sys->znode_list[cnt].u_stamp = -1;

			//increment the counter
			cnt ++;
		}

		fclose(fp);
	}
}

void sys_update_dev_install(sys_t *sys, char* install_file) {
	FILE* fp;
	int m;
	int len;
	int cnt;

	fp = fopen(FILE_INSTALL, "w+");

	if (fp == NULL) {
		return;
	}

	len = sys_get_znode_num(sys);
	cnt = 0;

	for (m = 0; m < ZNET_SIZE; m ++) {
		if (sys->znode_install_list[m].type > 0) {
			printf("save node into file, idx = %d, type = %d\n, descrip = %s", m, sys->znode_install_list[m].type, sys->znode_install_list[m].descrip);
			cnt++;
			fwrite(sys->znode_install_list[m].id, 8, sizeof(char), fp);
			fprintf(fp, ":");
			fprintf(fp, "%d", sys->znode_install_list[m].type);
			fprintf(fp, ":");
			fprintf(fp, "%s", sys->znode_install_list[m].descrip);
			fprintf(fp, ":");

			if (cnt < len) {
				fprintf(fp, "\n");
			}
		}
	}
	fclose(fp);
}

void sys_edit_dev_install(sys_t *sys, char id[8], int dev_type, int len_descrip, char* descrip){
	int m;
	int idx;
	int idxEmpty;
	int len = sys_get_znode_num(sys);

	if (len <= 0) {
		idx = 0;
	}
	else {
		idx = -1;
	}

	printf("edit install, dev id = %s, dev type = %d, sys znode num = %d \n", id, dev_type, len);
	//searching for device index
	for (m = 0; m < len; m ++){
		if (znode_isempty(& (sys->znode_list[m]) )) {
			continue;
		}
		else {
			if (!memcmp(sys->znode_install_list[m].id, id, sizeof(char)*8)){
				idx = m;
				break;
			}
		}
	}

	if (idx < 0) {
		idxEmpty = -1;
		for (m = 0; m < ZNET_SIZE; m ++) {
			if (znode_isempty(& (sys->znode_list[m]) )) {
				idxEmpty = m;
				break;
			}
		}

		//new device at the end of the list
		if (idxEmpty == -1 && len < ZNET_SIZE) {
			memcpy(sys->znode_install_list[len].id, id, sizeof(char)*8);
			sys->znode_install_list[len].type = dev_type;
			bzero(sys->znode_install_list[len].descrip, 50*sizeof(char));
			sys->znode_install_list[len].len_descrip = len_descrip;
			memcpy(sys->znode_install_list[len].descrip, descrip, len_descrip*sizeof(char));

			//add new znode into install and znode_install lists
			memcpy(sys->znode_list[len].id, id, sizeof(char)*8);
			sys->znode_list[len].type = dev_type;
			sys->znode_list[len].model = DEFAULT_MODEL;
			sys->znode_list[len].ver = DEFAULT_VER;
			sys->znode_list[len].status_len = get_znode_status_len(sys->znode_list[len].type);
			sys->znode_list[len].status = calloc(sys->znode_list[len].status_len, sizeof(char));
			sys->znode_list[len].znet_status = ZNET_ON;
			sys->znode_list[len].u_stamp = 0;

			printf("edit install, dev id = %s, dev type = %d, idx = %d\n", id, dev_type, len);
		}
		else if (idxEmpty > 0 ) {
			//new device at empty position in the list
			memcpy(sys->znode_install_list[idxEmpty].id, id, sizeof(char)*8);
			sys->znode_install_list[idxEmpty].type = dev_type;
			sys->znode_install_list[idxEmpty].len_descrip = len_descrip;
			bzero(sys->znode_install_list[len].descrip, 50*sizeof(char));
			memcpy(sys->znode_install_list[idxEmpty].descrip, descrip, len_descrip*sizeof(char));

			memcpy(sys->znode_list[idxEmpty].id, id, sizeof(char)*8);
			sys->znode_list[idxEmpty].type = dev_type;
			sys->znode_list[idxEmpty].model = DEFAULT_MODEL;
			sys->znode_list[idxEmpty].ver = DEFAULT_VER;
			sys->znode_list[idxEmpty].status_len = get_znode_status_len(sys->znode_list[idxEmpty].type);
			sys->znode_list[idxEmpty].status = calloc(sys->znode_list[idxEmpty].status_len, sizeof(char));
			sys->znode_list[idxEmpty].znet_status = ZNET_ON;
			sys->znode_list[idxEmpty].u_stamp = 0;

			printf("edit install, dev id = %s, dev type = %d, idx = %d\n", id, dev_type, idxEmpty);
		}
		else if (idxEmpty == -1 && len >= ZNET_SIZE) {
			return;
		}
	}
	else {
		//if device already in the installation list, simply renew the description
		memcpy(sys->znode_install_list[idx].id, id, sizeof(char)*8);
		sys->znode_install_list[idx].type = dev_type;
		sys->znode_install_list[idx].len_descrip = len_descrip;
		bzero(sys->znode_install_list[idx].descrip, 50*sizeof(char));
		memcpy(sys->znode_install_list[idx].descrip, descrip, len_descrip*sizeof(char));

		//add new znode into install and znode_install lists
		memcpy(sys->znode_list[idx].id, id, sizeof(char)*8);
		sys->znode_list[idx].type = dev_type;
		sys->znode_list[idx].model = DEFAULT_MODEL;
		sys->znode_list[idx].ver = DEFAULT_VER;
		sys->znode_list[idx].status_len = get_znode_status_len(sys->znode_list[idx].type);
		sys->znode_list[idx].status = calloc(sys->znode_list[idx].status_len, sizeof(char));
		sys->znode_list[idx].znet_status = ZNET_ON;
		sys->znode_list[idx].u_stamp = 0;

		printf("edit install, dev id = %s, dev type = %d, idx = %d\n", id, dev_type, idx);
	}
}

int sys_del_dev_install(sys_t *sys, char id[8]){
	int idx = -1;
	int m;

	for (m = 0; m < ZNET_SIZE; m ++){
		if (memcmp(sys->znode_install_list[m].id, id, sizeof(char)*8)){
			idx = m;
		}
	}

	if (idx < 0) {
		return idx;
	}
	else {

		//remove from znode_install list
		bzero(sys->znode_install_list[idx].descrip, 50*sizeof(char));
		memset((void*) &(sys->znode_install_list[idx]), 0, sizeof(znode_install));

		//remove from znode list
		free(sys->znode_list[idx].status);
		memset((void*) &(sys->znode_list[idx]), 0, sizeof(znode));
		return idx;
	}
}

int get_znode_status_len(int type){
	switch(type){
		case DEV_SWITCH_1:
			return 1;
		case DEV_SWITCH_2:
			return 2;
		case DEV_SWITCH_3:
			return 3;
		case DEV_SWITCH_4:
			return 4;
		case DEV_DIMSWITCH_1:
			return 1;
		case DEV_CURTAIN_1:
			return 1;
		case DEV_CURTAIN_2:
			return 2;
		case DEV_SOCK_1:
			return 1;
		case DEV_SOCK_4:
			return 4;
		case DEV_SENSOR_SMOKE:
			return 1;
		case DEV_SENSOR_CO:
			return 1;
		case DEV_SENSOR_WATER:
			return 1;
		case DEV_SENSOR_TEMP:
			return 2;
		case DEV_SENSOR_HUMI:
			return 2;
		case DEV_SENSOR_TEHU:
			return 2;
		case DEV_SENSOR_INFRA:
			return 8;
		case DEV_SENSOR_LUMI:
			return 2;
		case DEV_SENSOR_PM25:
			return 2;
		case DEV_SENSOR_MAGLOCK:
			return 1;
		case DEV_SENSOR_FLAME:
			return 1;
		case DEV_SENSOR_RAIN:
			return 1;
		case DEV_MECH_VALVE:
			return 1;
		case DEV_THEME_4:
			return 4;
		case DEV_INFRACTRL:
			return 8;
		case DEV_ALARM:
			return 1;
		default:
			return 0;
	}
}
