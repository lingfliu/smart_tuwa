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
    char status[SYS_LEN_STATUS]; //root status of the system

    //znet
    znode znode_list[PROC_ZNODE_MAX]; //fixed length of znode_list

    //stamp for synchronization
    long u_stamp;//sys stamp for status update
    long g_stamp;//extra sys stamp for satus update (not used yet)

    //stamp for tx msg
    int tx_msg_stamp;

    //license
    char lic[SYS_LEN_LIC];
    char cookie[SYS_LEN_COOKIE];
    int lic_status;

    //timer 
    struct timeval timer_pulse; //timer to send tcp pulse to the server
    struct timeval  timer_reset; //timer to reset the system 
    struct timeval  timer_sync; //timer to synchronization 

    //server status
    int server_status;
}system;

//system operation
void sys_get_id(system *sys, char* id_file); //get GW id from file

void sys_get_lic(system *sys, char* lic_file); //get license from file
int sys_init(system *sys); //initial empty system
void sys_reset(system *sys); //reset & flush the system to its initial status

int sys_znode_update(system* sys, message* msg); //update the znode status in system, if znode is not found, add into the znode_list, return the index of the updated znode

int sys_get_znode_idx(system* sys, char id[8]); //return the znode index given id

int sys_get_znode_num(system* sys); //get the actual number of znodes in the list
message* sys_sync(system *sys, message *msg); //synchronize the system by ack msg, return sync msg or NULL

void sys_save(system *sys, char* save_file); //save the current system status to file 
void sys_load(system *sys, char* save_file); //save the current system status to file 
#endif
