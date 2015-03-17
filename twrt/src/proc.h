#ifndef _PROC_
#define _PROC_

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "buffer_ex.h"

//////////////////////////////////////////////////
//device type
//////////////////////////////////////////////////
#define DEV_SWITCH_1 1
#define DEV_SWITCH_2 2
#define DEV_SWITCH_3 3
#define DEV_SWITCH_4 4
#define DEV_DIMSWITCH_1 5
#define DEV_CURTAIN_1 6
#define DEV_CURTAIN_2 7
#define DEV_SOCK_1 8

#define DEV_SENSOR_SMOKE 101
#define DEV_SENSOR_CO 102
#define DEV_SENSOR_WATER 103
#define DEV_SENSOR_TEMP 104
#define DEV_SENSOR_HUMI 105
#define DEV_SENSOR_TEHU 106
#define DEV_SENSOR_INFRA 107


#define DEV_MECH_VALVE 201 
#define DEV_THEME_4 202 
#define DEV_INFRACTRL 203


//////////////////////////////////////////////////
//data type
//////////////////////////////////////////////////

#define DATA_STAT 1 //status update, from znode to gateway
#define DATA_CTRL 2 //control, from znode to gateway, from gateway to znode, from server to gateway

#define DATA_REQ_SYNC 21 //synchronization request
#define DATA_REQ_AUTH_GW 22 //gw authroization request
#define DATA_REQ_AUTH_DEV 23 //dev authroization request
#define DATA_REQ_USER 24 //user list request
#define DATA_REQ_STAMP 25 //initial stamp request
#define DATA_REQ_PULSE 26 //tcp pulse 

#define DATA_ACK_SYNC 61 //synchronization ack
#define DATA_ACK_AUTH_GW 62 //auth gw ack
#define DATA_ACK_AUTH_DEV 63 //auth dev ack
#define DATA_ACK_USER 64 //user list ack
#define DATA_ACK_STAMP 65 //initial stamp ack
#define DATA_ACK_PULSE 66 //tcp pulse ack

#define DATA_SYS_RESET 101 //sys reset command

//////////////////////////////////////////////////
//data contents
//////////////////////////////////////////////////
#define STAT_ON  100 
#define STAT_OFF 0 

#define ERR_NET 0xF0
#define ERR_ELEC 0xF1
#define ERR_SYS 0xF2

#define TCP_PULSE 0xAA

#define AUTH_OK 0
#define AUTH_NO 0xFF

#define CTRL_ON 0 
#define CTRL_OFF 100 
//#define DATA_CTRL_LEVEL: range from 0~100 

#define THEME_SET 0x11


//////////////////////////////////////////////////
//define the header for the messages
//////////////////////////////////////////////////
#define HEADER_GW "\x0d\x0d\x0d\x0d" //for packets over gw server and znet, 4 bytes
#define HEADER_FE "\x0d" //for packets over znode and fe, 1 byte

//////////////////////////////////////////////////
//Protocol defined constants
//////////////////////////////////////////////////////////

//default model and versio
#define PROC_DEFAULT_MODEL 1
#define PROC_DEFAULT_VER 2

//other specifications
#define PROC_MAX_ZNODE 255 //maximum number of znodes supported by the znet

//header stamp id_gw id_znode znode_type data_type data_len data crc: 4+4+6+6+2+2+2+x+1
#define PROC_MSG_MIN 27//minimum length of message

//properties
#define MSG_TO_SERIAL 1
#define MSG_TO_INET_CLIENT 2
#define MSG_TO_INET_SEVER 3
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

struct_message* message_destroy(struct_message* msg);
struct_message* message_flush(struct_message *msg);//flush message without destroy it
struct_message* message_copy(struct_message *msg_dst, struct_message *msg_src);
int message_is_req(struct_message* msg);
int message_direction(struct_message* msg);
//message translation
struct_message* bytes2msg(buffer_byte_ring* bytes, struct_message* msg);
int msg2bytes(struct_message* msg, char** bytes);


//message q and operations
typedef struct struct_message_queue{
    struct_message* msg;
    struct_message_queue* prev;
    struct_message_queue* next;
    struct timeval timer;
}struct_message_queue;

void message_queue_init(struct_message_queue* msg_q);
void message_queue_put(struct_message_queue** msg_q, struct_message* msg);
struct_message* message_queue_get(struct_message_queue* msg_q, struct_message* msg);
void message_queue_del(struct_message_queue** msg_q, int len);//delete current message from the queue 
void message_queue_del_stamp(struct_message_queue** msg_q, long stamp);
int message_queue_has_stamp(struct_message_queue* msg_q, long stamp);
struct_message_queue* message_queue_move_head(struct_message_queue* msg_q);
struct_message_queue* message_queue_move_tail(struct_message_queue* msg_q);
int message_queue_len(struct_message_queue* msg_q);
int message_queue_is_empty(struct_message_queue* msg_q);

#endif
