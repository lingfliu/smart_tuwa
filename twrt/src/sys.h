#ifndef _SYS_
#define _SYS_

#include "config.h"
#include "proc.h"
#include "simple_inet.h"
#include "simple_serial.h"

#include <string.h>
#include <stdlib.h>

#define SERVER_CONNECT 1
#define SERVER_DISCONNECT 2
#define SERVER_NORESPONSE 3

#define SERIAL_ON 1
#define SERIAL_OFF 0

#define SYS_REG_NO 0
#define SYS_REG_OK 0

#define LIC_VALID 1
#define LIC_INVALID 2
#define LIC_UNKNOWN 3

//sys and znode
typedef struct{
    char id[8];
    int type;//type
    int model;//manufacturing model
    int ver;//firmware version
    int status_len;
    char* status;
    int net_status;
    long u_stamp;//node stamp for status update
    long g_stamp;//extra stamp for status update (not used yet)
}znode; 

int znode_isempty(znode *znd);
int znode_copy(znode* znode_dst, znode* znode_src);
void znode_create(znode* znd, message *msg); //create a znode given status msg
void znode_update(znode* znd, message *msg); //update a znode given status msg
void znode_flush(znode* znd);
//znode opeation
/////////////////////////////////////////////

typedef struct{
    //sys core
    char id[8];
    int model;//manufacturing model
    int ver;//firmware version
    char status[SYS_LEN_STATUS]; //root status of the sys_t

    //znet
    znode znode_list[PROC_ZNODE_MAX]; //fixed length of znode_list

    //stamp for synchronization
    long u_stamp;//sys stamp for status update
    long g_stamp;//extra sys stamp for satus update (not used yet)

    //stamp for tx msg
    long tx_msg_stamp;

    //license
    char lic[SYS_LEN_LIC];
    char cookie[SYS_LEN_COOKIE];
    int lic_status;

    //timer 
    struct timeval timer_pulse; //timer to send tcp pulse to the server
    struct timeval  timer_reset; //timer to reset the sys_t 
    struct timeval  timer_sync; //timer to synchronization 

    //server status
    int server_status;

	//serial status
	int serial_status;
}sys_t;

//sys_t operation
void sys_get_id(sys_t *sys, char* id_file); //get GW id from file

void sys_get_lic(sys_t *sys, char* lic_file); //get license from file
void sys_init(sys_t *sys); //initial empty sys_t
void sys_reset(sys_t *sys); //reset & flush the sys_t to its initial status

int sys_znode_update(sys_t* sys, message* msg); //update the znode status in sys_t, if znode is not found, add into the znode_list, return the index of the updated znode

int sys_get_znode_idx(sys_t* sys, char id[8]); //return the znode index given id

int sys_get_znode_num(sys_t* sys); //get the actual number of znodes in the list
message* sys_sync(sys_t *sys, message *msg); //synchronize the sys_t by ack msg, return sync msg or NULL

void sys_save(sys_t *sys, char* save_file); //save the current sys_t status to file 
void sys_load(sys_t *sys, char* save_file); //save the current sys_t status to file 
#endif
