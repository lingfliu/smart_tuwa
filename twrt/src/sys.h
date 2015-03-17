#ifndef _SYS_
#define _SYS_

#include "config.h"
#include "proc.h"
#include "simple_inet.h"
#include "simple_serial.h"

#include <string.h>

#define SYS_STATUS_LEN  50
#define SYS_LIC_LEN 128 
#define SYS_COOKIE_LEN 64

#define ZNET_ON 1
#define ZNET_OFF 0

#define MSG_SYS_AUTH_REQ 1
#define MSG_SYS_NET_HB 2
#define MSG_SYS_STAMP_REQ 2

//sys and znode
typedef struct{
    char parent_id[6];
    char id[6];
    int type;//type
    int model;//manufacturing model
    int ver;//firmware version
    int status_len;
    char* status;
    int status_znet;
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

    //stamp for msg
    int msg_tx_stamp;

    //license
    char lic[SYS_LIC_LEN];
    char cookie[SYS_COOKIE_LEN];
    int is_valid_lic;

    //timer 
    time_t timer_hb;
    time_t timer_reset;
}struct_sys;

//system initialization & reset
int sys_init(struct_sys* sys, struct_config* cfg);//initialize system as empty
int sys_flush(struct_sys* sys); //reset the system (keep basic paras)

//get id
void sys_get_id(struct_sys* sys);
//get licsense
void sys_get_lic(struct_sys* sys, char* file_lic);
//get authentification 
int sys_get_auth(struct_sys* sys, struct_inet* client);
//get stsamp
long sys_get_stamp(struct_sys* sys, struct_inet* client);

//znode operation 
int znode_copy(struct_znode* znode_dst, struct_znode* znode_src);
void znode_flush(struct_znode* znode);
int sys_znode_update(struct_sys* sys, struct_message* msg);

//synchronization
void sys_sync_znode(struct_sys* sys, int idx_znode, struct_message_queue** msg_q_tx);//synchronize the system znet status
void sys_sync_root(struct_sys* sys, struct_message_queue** msg_q_tx);//synchronize the system znet status

//network operation
int sys_net_hb(struct_message_queue** msg_q_tx);//heart beat to the server

//local save/load 
//int sys_save_sys(char* file_name);
//int sys_load_sys(char* file_name);

//time


//message creation
struct_message* message_create_empty(struct_message* msg, char gw_id[6]);
struct_message* message_create_sys(struct_message* msg, struct_sys* sys, int type, long* stamp);
struct_message* message_create_sync_znode(struct_znode* znode, struct_message* msg);
struct_message* message_create_sync_root(struct_sys* sys, struct_message* msg);
#endif
