#ifndef _PROC_
#define _PROC_

#include <string.h>
#include <stdlib.h>
#include "buffer_ex.h"

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
//status and ctrl
#define DATA_TYPE_STAT 1 //status update, from znode to gateway
#define DATA_TYPE_CTRL 2 //control, from znode to gateway, from gateway to znode, from server to gateway

#define DATA_TYPE_REQ_AUTH 20 //request, from gateway to server
#define DATA_TYPE_SYNC 21 //request, from gateway to server, from gateway to znet
#define DATA_TYPE_ACK_AUTH 50
#define DATA_TYPE_SYNC_STAT 51

//sys operation datum
#define DATA_TYPE_SYS_RST 101 //sys reset 
#define DATA_TYPE_NET_HB 201 //heart beat to server	


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

//define the header for the messages
#define PROC_DATA_HEADER "AAAA" //4 bytes

//default model and versio
#define PROC_DEFAULT_MODEL 1
#define PROC_DEFAULT_VER 2

//other specifications
#define PROC_MAX_ZNODE 255 //maximum number of znodes supported by the znet

//header stamp id_gw id_znode znode_type data_type data_len data crc: 4+4+6+6+2+2+2+x+1
#define PROC_MSG_MIN 27//minimum length of message

//////////////////////////////////////////
//structs and operations
//////////////////////////////////////////

//message
typedef struct{
    char gateway_id[6];
    char dev_id[6]; 
    int dev_type;
    int data_type;
    int data_len;
    char* data;
    long stamp;
}struct_message;

struct_message* message_copy(struct_message *msg_dst, struct_message *msg_src);
struct_message* message_flush(struct_message *msg);//flush message as empty message
struct_message* message_create(struct_message* msg, int type, long* stamp);
int message_is_ack(struct_message* msg);

//message translation
struct_message* bytes2msg(buffer_byte_ring* bytes, struct_message* msg);
char* msg2bytes(struct_message* msg, char* byte);


//message q and operations
typedef struct struct_message_queue{
    struct_message* msg;
    struct_message_queue* prev;
    struct_message_queue* next;
}struct_message_queue;

void message_queue_init(struct_message_queue* msg_q);
void message_queue_put(struct_message_queue** msg_q, struct_message* msg);
void message_queue_get(struct_message_queue* msg_q, struct_message* msg);
void message_queue_flush(struct_message_queue* msg_q, int len);
void message_queue_flush_stamp(struct_message_queue* msg_q, long stamp);
int message_queue_has_stamp(struct_message_queue* msg_q, long stamp);
int message_queue_getlen(struct_message_queue* msg_q);
int message_queue_isempty(struct_message_queue* msg_q);

#endif
