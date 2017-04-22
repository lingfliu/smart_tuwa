#ifndef _SYS_
#define _SYS_

#include "config.h"
#include "proc.h"
#include "simple_inet.h"
#include "simple_serial.h"
#include "buffer_ex.h"
#include "utils.h"
#include "gw_control.h"

#include <string.h>
#include <stdlib.h>
#include <pthread.h>

//sys status code
#define SERVER_CONNECT 1
#define SERVER_DISCONNECT 0

#define LOCALHOST_LISTENING 1
#define LOCALHOST_CLOSED 2 

#define ZNET_ON 1
#define ZNET_OFF 2

#define SERIAL_ON 1
#define SERIAL_OFF 2

#define SOCKET_CONNECT 1
#define SOCKET_DISCONNECT 2

//license status code
#define LIC_VALID 1
#define LIC_INVALID 2
#define LIC_UNKNOWN 3

//default auth code
#define DEFAULT_AUTHCODE "TWRT2015"
#define NULL_USER "00000000"

//system size
#define ZNET_SIZE 255 //maximum number of znodes in znet
#define LOCALUSER_SIZE 3 //maximum number of concurrent localuser connection 

#define LOCAL_STATUS_MSGINVALID 'a'
#define LOCAL_STATUS_SKTDISCONNECT 'b'
#define LOCAL_STATUS_EXITNORMAL 'c'

//memory settings
#define BUFF_IO_LEN 20000 
#define BUFF_RING_LEN 20000 
#define MSG_Q_LEN 50  //length of message queue

//timer settings
#define TIMER_PULSE 1000 //pulse ack waiting time in milli-second
/*new code, timer pulse ack*/
#define TIMER_PULSE_ACK 50000 //pulse ack waiting time in milli-second
#define TIMER_RESET 72 //pulse ack waiting time in hour 
#define TIMER_BAKUP 1800 //bakup time in seconds ( 30 minutes by default)
#define TIMER_REQ 2000 //request waiting time in milli-second
#define TIMER_SYNC 30 //synchronization time in seconds 

#define DEFAULT_LOCALHOST_TIMEOUT 5 //server socket timeout in seconds

//sys and znode
typedef struct{
    char id[8];
    int type;//type
    int model;//manufacturing model
    int ver;//firmware version
    int status_len;
    char* status;
    int znet_status; //znet status: on, off
    long u_stamp;//node stamp for status update
    long g_stamp;//extra stamp for status update (not used yet)
}znode; 

//znode operation
int znode_isempty(znode *znd); //check if znode contains no znode info
int znode_copy(znode* znode_dst, znode* znode_src); //hard copy of a znode
void znode_create(znode* znd, message *msg); //create a znode given status msg
void znode_update(znode* znd, message *msg); //update a znode given status msg
void znode_delete(znode* znd); //delete the znode (both status and other infos)
void znode_flush(znode* znd); //flush the znode's status and u_stamp

//znode operations
int get_znode_status_len(int type);
//local client
typedef struct{
	char id[8];
	int skt;
	pthread_t thrd_rx;
	pthread_t thrd_tx;
	char buff_io[BUFF_IO_LEN];
	buffer_ring_byte buff;
	int is_authed;
    struct timeval time_lastactive; //timer to send tcp pulse to the server
	char tx_status;

	/***********************
	  debug codec
	 ***********************/
	int idx;
}localuser;

int localuser_isnull(localuser *usr);
int localuser_create(localuser *usr, char id[8], int skt);
void localuser_delete(localuser *usr);

//bundle for tx message of localuser
typedef struct{
	localuser *usr;
	message *msg;
}localbundle;


#define POS_TYPE_INT 1
#define POS_TYPE_EXT 2

//znode install 
typedef struct{
	char id[8];
	int type;
	char name[60];
	char pos[60];
	int  posType;
}znode_install;

typedef struct{
	char id[8];
	char state[32];
	int state_len;
	int type;
}scene_item;

#define SCENE_TYPE_HARD 1
#define SCENE_TYPE_SOFT 2
#define SCENE_TYPE_TRIGGER 3
#define MAX_SCENE_NUM 100

typedef struct{
	char host_mac[8];
	char host_id_major[8];
	char host_id_minor[8];

	int scene_type;
	char scene_name[60];

	int trigger_num;
	int item_num;

	scene_item* trigger; //arrays
	scene_item* item; //arrays
}scene;

/////////////////////////////////////////////
typedef struct{
    //sys core
    char id[8];
    int model;//manufacturing model
    int ver;//firmware version
    char status[SYS_LEN_STATUS]; //root status of the sys_t

    //znet
    znode znode_list[ZNET_SIZE]; //fixed length of znode_list
	znode_install znode_install_list[ZNET_SIZE];

	scene sces[MAX_SCENE_NUM];

	//local client
	localuser localuser_list[LOCALUSER_SIZE];

    //stamp for synchronization
    long u_stamp;//sys stamp for status update
    long g_stamp;//extra sys stamp for satus update (not used yet)

	//counters
    long tx_msg_stamp;//stamp for tx_msg

    //license
    char lic[SYS_LEN_LIC]; //lic should be a string code to activate the GW in the server
    char auth_code[SYS_LEN_AUTHCODE]; //cookie should be a short auth code for GW identification and local operation
    int lic_status;

    //timer 
    struct timeval timer_pulse; //timer to send tcp pulse to the server
    struct timeval timer_pulse_ack; //timer to send tcp pulse to the server
    struct timeval  timer_reset; //timer to reset the sys_t (not used)
    struct timeval  timer_sync; //timer to synchronization 
	struct timeval timer_bakup;

	//connection status
    int server_status;//server status
	int serial_status;//serial status
	int local_status;//localhost status

	/*
	 * status of GW devices
	 */
	char fan_status; //fan status
}sys_t;


void sys_get_id(sys_t *sys, char* id_file); //get GW id from file
void sys_get_lic(sys_t *sys, char* lic_file); //get license from file

//new codec for device install & scene install
//installs will be used as whitelist
void sys_get_dev_install(sys_t *sys, char* install_file); 
void sys_update_dev_install(sys_t *sys, char* install_file);
int sys_edit_dev_install(sys_t *sys, znode_install* install);
int sys_del_dev_install(sys_t *sys, char id[8]);


void sys_get_scene(sys_t *sys, char* scene_file); //read scenes into file
void sys_update_scene(sys_t *sys, char* scene_file); //store scenes into file
int sys_edit_scene(sys_t* sys, scene* sce); //modify scene
int sys_del_scene(sys_t* sys, char id_major[8], char id_minor[8]); //delete a scene
scene* sys_find_scene(sys_t* sys, char id_major[8], char id_minor[8]);
scene* sys_find_scene_bymac(sys_t* sys, char dev_mac[8], char id_minor[8]);
scene* sys_find_scene_bytrigger(sys_t* sys, char trigger_id[8], char trigger_state[8]);
int sys_get_scene_num(sys_t* sys);

/*system initialization when program started
todo:
 1. get id, lic
 2. initialize an znode_list fro FILE_INSTALL
 3. initialize an empty localuser_list
 4. set tx_msg_stamp = 0
 5. set u_stamp = -1
 6. init timers as gettimeofday
 7. set connection status as disconnected & close*/
void sys_init(sys_t *sys); 

/*synchronize the sys_t based on sync msg, return sync msg (sync to server) or NULL*/
message* sys_sync(sys_t *sys, message *msg); 

//znode operation
int sys_znode_update(sys_t* sys, message* msg); //update the znode status in sys_t, if znode is not found, add into the znode_list, return the index of the updated znode
int sys_get_znode_idx(sys_t* sys, char id[8]); //return the znode index given id
int sys_get_znode_num(sys_t* sys); //get the actual number of znodes in the list

//localuser operation
localuser* sys_localuser_login(sys_t* sys, int skt); 
int sys_localuser_logout(sys_t* sys, char id[8]); 
int sys_localuser_evict(sys_t* sys); //force logout (passive logout) a localuser if: 1. localuser_list is full, 2. socket do not receive any data after timeout
int sys_get_localuser_idx(sys_t* sys, char id[8]);
int sys_get_localuser_num(sys_t* sys);


/*
 * new funcs for scene
 */
message* message_create_scene(char id_gw[8], scene* sce);
message* message_create_install_adv(char id_gw[8], znode_install* install);
znode_install* sys_find_install(sys_t* sys, char id[8]);

/*
 * new funcs for sys synchronization
 */
void sys_znet_bakup(sys_t* sys, char* bakup_file);
void sys_znet_restore(sys_t* sys, char* bakup_file);

#endif
