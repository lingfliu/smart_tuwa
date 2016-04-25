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

	//initialize znet install
	/* removed */
	sys_get_dev_install(sys, FILE_INSTALL);

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
	 * initialize scene
	 */
	sys_get_scene(sys, FILE_SCENE); //read scenes into file

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
		   //TODO: add unversioned node into the system list
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
	int read_len = 136;

	FILE* fp;
	//char str[256];
	//char strTmp[256];
	int cnt;
	int m;
	//int idx[3];
	//int idxCnt;
	//int len_descrip;
	znode_install tmp_ins;

	fp = fopen(install_file,"r");
	if(fp == NULL){
		printf("install file cannot be opened\n");
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

		while (fread(&(tmp_ins), sizeof(znode_install), 1, fp) > 0){
			//printf("read install idx = %d\n", cnt);
			memcpy(&(sys->znode_install_list[cnt]), &tmp_ins, sizeof(znode_install));
			cnt ++;
		}
		/*
		while(fgets(str,256,fp)!=NULL) {
			memcpy(sys->znode_install_list[cnt].id, str, 8*sizeof(char));
			memcpy(&(sys->znode_install_list[cnt].type), str+8, sizeof(int));
			memcpy(sys->znode_install_list[cnt].name, str+12, 60*sizeof(char));
			memcpy(sys->znode_install_list[cnt].pos, str+72, 60*sizeof(char));
			memcpy(&(sys->znode_install_list[cnt].posType), str+132, sizeof(int));
			cnt ++;
		}
		*/
		fclose(fp);
	}
}

void sys_update_dev_install(sys_t *sys, char* install_file) {
	FILE* fp;
	int m;
	int len;
	int cnt;

	fp = fopen(install_file, "w+");

	if (fp == NULL) {
		return;
	}
	else{
		for (m = 0; m < ZNET_SIZE; m ++) {
			if (sys->znode_install_list[m].type <= 0) break;

			printf("save node into file, idx = %d, type = %d\n, name = %s\n", m, sys->znode_install_list[m].type, sys->znode_install_list[m].name);

			if (fwrite(&(sys->znode_install_list[m]), 1, sizeof(znode_install), fp) < 0){
				printf("write failed, quit\n");
				break;
			}
		}
		fclose(fp);
	}
}

int sys_edit_dev_install(sys_t *sys, znode_install* install){
	int m;
	int is_update = -1;
	int retval = -1;
	int idx = -1;
	for (m = 0; m < ZNET_SIZE; m ++){
		if (sys->znode_install_list[m].type <= 0) {
			//found a position for the new znode_install
			retval = 0;
			idx = m;
			break;
		}
		else if(!memcmp(install->id, sys->znode_install_list[m].id, 8)){
			//update old znode
			memcpy(sys->znode_install_list[m].name, install->name, 60*sizeof(char));
			memcpy(sys->znode_install_list[m].pos, install->pos, 60*sizeof(char));
			sys->znode_install_list[m].posType = install->posType;

			is_update = 1;
			retval = 0;
			printf("edit old install, dev id = %s, idx = %d\n", sys->znode_install_list[m].id, m);
			break;
		}
	}

	//new znode_install
	if (is_update < 0 && idx < ZNET_SIZE) {
		//update old znode
		memcpy(sys->znode_install_list[idx].id, install->id, 8*sizeof(char));
		memcpy(sys->znode_install_list[idx].name, install->name, 60*sizeof(char));
		memcpy(sys->znode_install_list[idx].pos, install->pos, 60*sizeof(char));
		sys->znode_install_list[idx].posType = install->posType;
		sys->znode_install_list[idx].type = install->type;
		retval = 0;
		printf("edit new install, dev id = %s, idx = %d\n", install->id, m);
	}
	return retval;
}

int sys_del_dev_install(sys_t *sys, char id[8]){
	int m;
	int n;
	int retval = -1;
	for (m = 0; m < ZNET_SIZE; m ++){
		if(!memcmp(id, sys->znode_install_list[m].id, 8)) {
			retval = 0;
			memset(&(sys->znode_install_list[m]), 0, sizeof(znode_install));

			if (m > 0){
				//shrink the scene list
				for(n = m+1; m < ZNET_SIZE; n ++){
					if (sys->znode_install_list[n].type<= 0) break;
					memcpy(sys->znode_install_list[n-1].id, sys->znode_install_list[n].id, 8*sizeof(char));
					memcpy(sys->znode_install_list[n-1].name, sys->znode_install_list[n].name, 60*sizeof(char));
					memcpy(sys->znode_install_list[n-1].pos, sys->znode_install_list[n].pos, 60*sizeof(char));
					sys->znode_install_list[n-1].type = sys->znode_install_list[n].type;
					sys->znode_install_list[n-1].posType = sys->znode_install_list[n].posType;
				}
				bzero(&(sys->znode_install_list[n-1]), sizeof(znode_install));
			}
			break;
		}
		else if (sys->znode_install_list[m].type <= 0){
			break;
		}
	}
	return retval;
}

void sys_get_scene(sys_t *sys, char* scene_file){ //read scenes into file
	FILE* fp;
	//char str[4096];
	//char strTmp[4096];
	int cnt;
	int item_num;
	int trigger_num;
	int m;
	int idxCnt;
	scene tmp_sce;

	fp = fopen(scene_file,"r");
	if(fp == NULL){
		printf("file cannot be opened\n");
		//if file not existing, create one
		fp = fopen(FILE_SCENE, "w+");
		if (fp != NULL){
			fclose(fp);
		}
		return;
	}
	else {
		printf("read file: %s\n", FILE_SCENE);
		cnt = 0;

		while (fread(&(tmp_sce), sizeof(scene), 1, fp) > 0){
			if(cnt >= MAX_SCENE_NUM) break;

			item_num = tmp_sce.item_num;
			trigger_num = tmp_sce.trigger_num;
			tmp_sce.trigger = calloc(trigger_num, sizeof(scene_item));
			tmp_sce.item = calloc(item_num, sizeof(scene_item));

			if (trigger_num > 0) {
				if(fread(tmp_sce.trigger, sizeof(scene_item), trigger_num, fp) <= 0){
					break;
				}
			}

			if (item_num > 0) {
				if(fread(tmp_sce.item, sizeof(scene_item), item_num, fp) <= 0){
					break;
				}
			}

			//if all configuration read sucessful, copy to sys
			memcpy(&(sys->sces[cnt]), &tmp_sce, sizeof(scene));
			sys->sces[cnt].trigger = tmp_sce.trigger;
			sys->sces[cnt].item = tmp_sce.item;

			cnt ++;
		}

		fclose(fp);
	}
}

void sys_update_scene(sys_t *sys, char* scene_file){


	FILE* fp;
	int m;

	printf("update_scene\n");
	fp = fopen(scene_file, "w+");

	if (fp == NULL) {
		printf("failed to open scene file\n");
		return;
	}
	else {
		for (m = 0; m < MAX_SCENE_NUM; m ++){
			if (sys->sces[m].scene_type <= 0) break;
			
			printf("save scene into file, idx = %d, type = %d, trigger_num = %d, item_num = %d\n", m, sys->sces[m].scene_type, sys->sces[m].trigger_num, sys->sces[m].item_num);
			fwrite(&(sys->sces[m]), 1, sizeof(scene), fp);
			if (sys->sces[m].trigger_num > 0){
				if (fwrite(sys->sces[m].trigger, sys->sces[m].trigger_num, sizeof(scene_item),fp) <= 0){
					break;
				}
			}
			if (sys->sces[m].item_num > 0){
				if (fwrite(sys->sces[m].item, sys->sces[m].item_num, sizeof(scene_item),fp) <= 0){
					break;
				}
			}
		}
		fclose(fp);
	}
}

int sys_edit_scene(sys_t* sys, scene* sce){
	int m,n;
	int idx = -1;
	int is_update = -1;
	int retval = -1;
	for (m = 0; m < MAX_SCENE_NUM; m ++){
		if (sys->sces[m].scene_type <= 0) {
			//found a position for the new scene
			retval = 0;
			idx = m;
			//printf("found new position for scene=%d\n",idx);
			break;
		}
		else if(!memcmp(sce->host_id_major, sys->sces[m].host_id_major, 8) && !memcmp(sce->host_id_minor, sys->sces[m].host_id_minor, 8) ){
			//update old scene
			sys->sces[m].trigger_num = sce->trigger_num;
			sys->sces[m].item_num= sce->item_num;
			free(sys->sces[m].trigger);
			free(sys->sces[m].item);

			if (sys->sces[m].trigger_num > 0)
				sys->sces[m].trigger = calloc(sys->sces[m].trigger_num, sizeof(scene_item));
			if (sys->sces[m].item_num > 0)
				sys->sces[m].item = calloc(sys->sces[m].item_num, sizeof(scene_item));

			for (n = 0; n < sys->sces[m].trigger_num; n ++) 
				memcpy(&(sys->sces[m].trigger[n]), &(sce->trigger[n]), sizeof(scene_item));
			
			for (n = 0; n < sys->sces[m].item_num; n ++) 
				memcpy(&(sys->sces[m].item[n]), &(sce->item[n]), sizeof(scene_item));

			is_update = 1;
			retval = 0;
			break;
		}
	}

	//new scene
	if (is_update < 0 && idx < MAX_SCENE_NUM) {
		printf("found new position for scene=%d\n",idx);
		memcpy(sys->sces[idx].host_id_major, sce->host_id_major, sizeof(char)*8);
		memcpy(sys->sces[idx].host_id_minor, sce->host_id_minor, sizeof(char)*8);
		memcpy(sys->sces[idx].host_mac, sce->host_mac, sizeof(char)*8);

		sys->sces[idx].scene_type = sce->scene_type;
		memcpy(sys->sces[idx].scene_name, sce->scene_name, sizeof(char)*60);

		sys->sces[idx].trigger_num = sce->trigger_num;
		sys->sces[idx].item_num = sce->item_num;


		if (sys->sces[idx].trigger_num > 0)
			sys->sces[idx].trigger = calloc(sys->sces[idx].trigger_num, sizeof(scene_item));
		if (sys->sces[idx].item_num > 0)
			sys->sces[idx].item = calloc(sys->sces[idx].item_num, sizeof(scene_item));

		printf("runs here, trigger %d, item %d\n", sce->trigger_num, sce->item_num);

		for (n = 0; n < sys->sces[idx].trigger_num; n ++) 
			memcpy(&(sys->sces[idx].trigger[n]), &(sce->trigger[n]), sizeof(scene_item));

		for (n = 0; n < sys->sces[idx].item_num; n ++) 
			memcpy(&(sys->sces[idx].item[n]), &(sce->item[n]), sizeof(scene_item));

	}
	return retval;
}

int sys_del_scene(sys_t* sys, char id_major[8], char id_minor[8]){
	int m;
	int n;
	int retval = -1;
	for (m = 0; m < MAX_SCENE_NUM; m ++){
		if(!memcmp(id_major, sys->sces[m].host_id_major, 8) && !memcmp(id_minor, sys->sces[m].host_id_minor, 8) ){
			retval = 0;
			free(sys->sces[m].trigger);
			free(sys->sces[m].item);
			memset(&(sys->sces[m]), 0, sizeof(scene));

			printf("found delete scene = %d\n", m);
			if (m > 0){
				//shrink the scene list
				for(n = m+1; n < MAX_SCENE_NUM; n ++){
					if (sys->sces[n].scene_type <=0) break; //reach empty position

					printf("moving from %d to %d\n", n, n-1);
					memcpy(sys->sces[n-1].host_mac, sys->sces[n].host_mac, 8*sizeof(char));
					memcpy(sys->sces[n-1].host_id_major, sys->sces[n].host_id_major, 8*sizeof(char));
					memcpy(sys->sces[n-1].host_id_minor, sys->sces[n].host_id_minor, 8*sizeof(char));
					memcpy(sys->sces[n-1].scene_name, sys->sces[n].scene_name, 60*sizeof(char));
					sys->sces[n-1].scene_type = sys->sces[n].scene_type;
					sys->sces[n-1].trigger_num = sys->sces[n].trigger_num;
					sys->sces[n-1].item_num = sys->sces[n].item_num;
					sys->sces[n-1].trigger = sys->sces[n].trigger;
					sys->sces[n-1].item = sys->sces[n].item;
				}

				bzero(&(sys->sces[n-1]), sizeof(scene)); //remove last scene (updated to n-2)
			}
			break;
		}
	}

	return retval;
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

message* message_create_scene(char id_gw[8], scene* sce){
	int m;
	int msg_len  = 96+sce->trigger_num*16+sce->item_num*16;
	message *msg = message_create();
	memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
	memcpy(msg->dev_id, NULL_DEV, MSG_LEN_ID_DEV);
	msg->dev_type = 0;
	msg->data_type = DATA_SCENE;

	msg->data_len = msg_len;
	msg->data = realloc(msg->data, msg_len*sizeof(char));


	if (sce->scene_type == SCENE_TYPE_HARD)
		memcpy(msg->data, sce->host_mac, sizeof(char)*8);
	memcpy(msg->data+8, sce->host_id_major, sizeof(char)*8);
	memcpy(msg->data+16, sce->host_id_minor, sizeof(char)*8);
	memcpy(msg->data+24, &(sce->scene_type), sizeof(int));
	memcpy(msg->data+28, sce->scene_name, sizeof(char)*60);
	memcpy(msg->data+88, &(sce->trigger_num), sizeof(int));
	memcpy(msg->data+92, &(sce->item_num), sizeof(int));
	for (m = 0; m < sce->trigger_num; m ++)
		memcpy(msg->data+96+16*m, &(sce->trigger[m]), sizeof(scene_item));
	for (m = 0; m < sce->item_num; m ++) 
		memcpy(msg->data+96+16*sce->trigger_num+16*m, &(sce->item[m]), sizeof(scene_item));

	return msg;
}

scene* sys_find_scene(sys_t* sys, char id_major[8], char id_minor[8]){
	int m;
	scene* sce = NULL;
	for (m = 0; m < MAX_SCENE_NUM; m ++){
		//printf("id_major =%s, id_minor=%s\n", sys->sces[m].host_id_major, sys->sces[m].host_id_minor);
		if(!memcmp(id_major, sys->sces[m].host_id_major, 8) && !memcmp(id_minor, sys->sces[m].host_id_minor, 8) ){
			//printf("found scene\n");
			sce = &(sys->sces[m]);
		}
	}

	return sce;
}
scene* sys_find_scene_bymac(sys_t* sys, char dev_mac[8], char id_minor[8]){
	int m;
	scene* sce = NULL;
	for (m = 0; m < MAX_SCENE_NUM; m ++){
		if(!memcmp(dev_mac, sys->sces[m].host_mac, 8) && !memcmp(id_minor, sys->sces[m].host_id_minor, 8) ){
			sce = &(sys->sces[m]);
		}
	}

	return sce;
}
scene* sys_find_scene_bytrigger(sys_t* sys, char trigger_id[8], char trigger_state[8]){
	scene * sce = NULL;
	int m,n;

	for (m = 0; m < MAX_SCENE_NUM; m ++){
		if (sys->sces[m].scene_type <= 0) 
			break;
		for (n = 0; n < sys->sces[m].trigger_num; n ++){
			if(!memcmp(trigger_id, sys->sces[m].trigger[n].id, 8) && !memcmp(trigger_state, sys->sces[m].trigger[n].state, 8) ){
				sce = &(sys->sces[m]);
				break;
			}
		}
	}

	return sce;
}


message* message_create_install_adv(char id_gw[8], znode_install* install){
	int m;
	int msg_len  = 136; 
	message *msg = message_create();
	memcpy(msg->gateway_id, id_gw, MSG_LEN_ID_GW);
	memcpy(msg->dev_id, NULL_DEV, MSG_LEN_ID_DEV);
	msg->dev_type = 0;
	msg->data_type = DATA_INSTALL;

	msg->data_len = msg_len;
	msg->data = realloc(msg->data, msg_len*sizeof(char));


	memcpy(msg->data, install->id, 8*sizeof(char));
	memcpy(msg->data+8, &(install->type), sizeof(int));
	memcpy(msg->data+12, install->name, 60*sizeof(char));
	memcpy(msg->data+72, install->pos, 60*sizeof(char));
	memcpy(msg->data+132, &(install->posType), sizeof(int));

	return msg;
}

znode_install* sys_find_install(sys_t* sys, char id[8]){
	int m;
	znode_install* install = NULL;
	for (m = 0; m < ZNET_SIZE; m ++){
		if(!memcmp(id, sys->znode_install_list[m].id, 8)){
			install = &(sys->znode_install_list[m]);
			break;
		}
	}

	return install;
}

int sys_get_scene_num(sys_t* sys){
	int m;
	int cnt = 0;
	for (m = 0; m < MAX_SCENE_NUM; m ++){
		if (sys->sces[m].scene_type > 0)
			cnt ++;
		else
			break;
	}

	return cnt;
}
