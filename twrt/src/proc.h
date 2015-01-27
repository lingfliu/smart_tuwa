#ifndef _PROC_
#define _PROC_

#include <string.h>
#include <stdlib.h>

//Code list

////////device type
#define DEV_TYPE_SWITCH_SINGLE 1
#define DEV_TYPE_SWITCH_DOUBLE 2
#define DEV_TYPE_SWITCH_TRIPLE 3
#define DEV_TYPE_SWITCH_DIMMABLE_SINGLE 4
#define DEV_TYPE_CURTAIN_SINGLE 5
#define DEV_TYPE_CURTAIN_DOUBLE 6
#define DEV_TYPE_MECH 7

#define DEV_TYPE_SENSOR_SMOKE 101
#define DEV_TYPE_SENSOR_CO 102
#define DEV_TYPE_SENSOR_WATER 103
#define DEV_TYPE_SENSOR_INFRA 104
#define DEV_TYPE_SENSOR_TEMP 105
#define DEV_TYPE_SENSOR_HUMI 106
#define DEV_TYPE_SENSOR_TEMPHUMI 107

#define DEV_TYPE_INFRACTRL 201

#define DEV_TYPE_THEME_QUADRO 202

////////data type
#define DATA_TYPE_STAT 1 //status update, from znode to gateway
#define DATA_TYPE_CTRL 2 //control, from znode to gateway, from gateway to znode, from server to gateway
#define DATA_TYPE_REQ 3 //request, from gateway to server, from gateway to znet

//sys operation datum
#define DATA_TYPE_SYS_RST 101 //sys reset 

#define DATA_TYPE_SYS_NET_HB 201 //heart beat to server	


////////data contents
#define DATA_STAT_ON  0xFF
#define DATA_STAT_OFF 0x00
//#define DATA_STAT_LEVEL: range from 0~100
#define DATA_STAT_ERR 0xAA

#define DATA_CTRL_ON 0xFF
#define DATA_CTRL_OFF 0x00
//#define DATA_CTRL_LEVEL: range from 0~100 

#define DATA_REQ_SET 0x11
#define DATA_REQ_UNSET 0x22

#define DATA_NET_HB  0xAA
#define DATA_NET_ACK 0x00

//default model and ver
#define PROC_DEFAULT_MOD 1
#define PROC_DEFAULT_VER 2

//other specifications
#define PROC_MAX_ZNODE 255

//////////////////////////////////////////
//structs and operations
//////////////////////////////////////////

//message
typedef struct{
    char[6] id_gateway;
    char[6] id_dev;
    int dev_type;
    int data_type;
    int data_len;
    char* data;
}struct_message;

void copy_message(struct_message *msg_dst, struct_message *msg_src);
void flush_message(struct_message *msg);//flush message as empty message


//message queue and operations
typedef struct{
    struct_message* msg;
    struct_message_queue* prev;
    struct_message_queue* next;
    struct_message_queue* head;
    struct_message_queue* tail;
}struct_message_queue;

void init_message_queue(struct_message_queu* msg_queue);
void put_message_queue(struct_message_queue* msg_queue, struct_message* msg);
void get_message_queue(struct_message_queue* msg_queue, struct_message* msg);
void flush_message_queue(struct_message_queue* msg_queue, int len);
void flush_message_queue_single(struct_message_queue* msg_queue);

//sys and znode
typedef struct{
    char[6] id;
    char[6] id_parent;
    int type;//type
    int mod;//manufacturing model
    int ver;//firmware version
    char* status; 
    long u_stamp;//node stamp for status update
    long g_stamp;//extra stamp for status update
}struct_znode; 

typedef struct{
    char[6] id_gateway;
    int mod;//manufacturing model
    int ver;//firmware version
    //znet info
    int znode_num;//actual  znode number
    struct_znode znode_list[PROC_MAX_ZNODE];
    long u_stamp;//sys stamp for status update
    long g_stamp;//extra sys stamp for satus update
}struct_sys;

//destroy node in the list
void destroy_znode(struct_znode* znode);
void destroy_sys_status(struct_znode* znode);

//stamp operation
void renew_stamp(struct_sys* sys);
//void update_u_stamp(struct_sys* sys);
//void reset_stamp_znode(struct_sys* sys);
//void reset_stamp_sys(struct_sys* sys);

//sys status saving
int save_sys_status(struct_sys* sys, char* file_name);

//data translation & retro-translation
message* bytes2msg(char* bytes, struct_message* msg);
char* msg2bytes(struct_message* msg, char* bytes);

#endif
