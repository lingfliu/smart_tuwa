#ifndef _SYS_
#define _SYS_

#include "config.h"
#include "proc.h"
#include "simple_inet.h"
#include "simple_serial.h"

#include <string.h>

#define SYS_SUCCESS 1
#define SYS_FAIL 2
#define SYS_STATUS_LEN 255
#define SYS_LISCENSE_LEN 64

//sys and znode
typedef struct{
    char parent_id[6];
    char id[6];
    int type;//type
    int model;//manufacturing model
    int ver;//firmware version
    int status_len;
    char* status;
    long u_stamp;//node stamp for status update
    long g_stamp;//extra stamp for status update (not used yet)
}struct_znode; 

typedef struct{
    //sys core
    char id[6];
    int model;//manufacturing model
    int ver;//firmware version
    char status[SYS_STATUS_LEN]; //root status of the system

    //znet
    int znode_num;//actual  znode number
    struct_znode znode_list[PROC_MAX_ZNODE]; //fixed length of znode_list

    //stamp for synchronization
    long u_stamp;//sys stamp for status update
    long g_stamp;//extra sys stamp for satus update (not used yet)

    //liscense
    char liscense[SYS_LISCENSE_LEN];
    int is_valid_liscense;

    //time for 
    time_t timer;
}struct_sys;

//system operation
//system initialization & reset
int sys_init(struct_sys* sys);//allocate memory space for sys
int sys_reset(struct_sys* sys); //reset the system
void sys_flush(struct_sys* sys); //flush the system for reset

//znode operation 
//int znode_copy(struct_znode* znode_dst, struct_znode* znode_src);
//void znode_flush(struct_znode* znode);
//int sys_znode_put(struct_sys* sys, struct_znode* znode);
//int sys_znode_get(struct_sys* sys, char znode_id[6], struct_znode* znode);
int sys_znode_update(struct_sys* sys, struct_message* msg);

//synchronization
int sys_sync_znet(struct_sys* sys, int idx_znode, int socket);//synchronize the system znet status
int sys_sync_root(struct_sys* sys, int idx_znode, int socket);//synchronize the system znet status

//network operation
int sys_net_hb(int socket);//heart beat to the server
int sys_net_auth(struct_sys* sys, int socket);//authentication of the gateway

//local save/load 
int sys_save_sys(char* file_name);
int sys_load_sys(char* file_name);

//time
int sys_timer_update();
#endif
