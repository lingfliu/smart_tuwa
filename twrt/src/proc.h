#ifndef _PROC_
#define _PROC_

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
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
#define DEV_SOCK_4 9

#define DEV_SENSOR_SMOKE 101
#define DEV_SENSOR_CO 102
#define DEV_SENSOR_WATER 103
#define DEV_SENSOR_TEMP 104
#define DEV_SENSOR_HUMI 105
#define DEV_SENSOR_TEHU 106
#define DEV_SENSOR_INFRA 107
#define DEV_SENSOR_LUMI 108
#define DEV_SENSOR_PM25 109
#define DEV_SENSOR_MAGLOCK 110
#define DEV_SENSOR_FLAME 111
#define DEV_SENSOR_RAIN 112

#define DEV_MECH_VALVE 201 
#define DEV_THEME_4 202 
#define DEV_INFRACTRL 203
#define DEV_ALARM 204


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
//define the header of the packet
//////////////////////////////////////////////////
#define MSG_HEADER_GW "\x0d\x0d\x0d\x0d" //for packets over gw server and znet, 4 bytes
#define MSG_HEADER_FE "\x0d" //for packets over znode and fe, 1 byte

//////////////////////////////////////////////////
//Protocol defined constants
//////////////////////////////////////////////////

//default model and version for gw and dev
#define PROC_DEFAULT_MODEL 1
#define PROC_DEFAULT_VER 2

//other specifications
#define PROC_ZNODE_MAX 255 //maximum number of znodes supported by the znet

//header stamp id_gw id_dev dev_type data_type data_len data: 4+4+8+8+2+2+2+x
#define MSG_LEN_MIN 30//minimum length of a message
//message length contents
#define MSG_LEN_HEADER_GW 4 
#define MSG_LEN_HEADER_FE 1 
#define MSG_LEN_STAMP 4 
#define MSG_LEN_ID_GW 8 
#define MSG_LEN_ID_DEV 8 
#define MSG_LEN_DEV_TYPE 2 
#define MSG_LEN_DATA_TYPE 2 
#define MSG_LEN_DATA_LEN 2 

#define MSG_LEN_FIXED 30 

#define MSG_POS_STAMP 4
#define MSG_POS_ID_GW 8
#define MSG_POS_ID_DEV 16
#define MSG_POS_DEV_TYPE 24
#define MSG_POS_DATA_TYPE 26
#define MSG_POS_DATA_LEN 28

//properties
#define MSG_TO_ZNET 1
#define MSG_TO_SERVER 2
#define MSG_TO_LOCAL_DEV 3
#define MSG_TO_LOCAL_CLIENT 4

//twrt system parameters
#define SYS_LEN_STATUS 50 //status length of gw
#define SYS_LEN_LIC 128  //lic string length
#define SYS_LEN_COOKIE 32  //cookie length
#define ZNET_ON 1  //znet status of znode
#define ZNET_OFF 0 //znet status of znode
#define DEFAULT_VER 1 //default version of twrt
#define DEFAULT_MODEL 1 //default vendor of all devices

#define NULL_DEV "00000000" 
//////////////////////////////////////////
//structs and operations
//////////////////////////////////////////

//message
//////////////////////////////////////////
typedef struct{
    char gateway_id[8];
    char dev_id[8]; 
    int dev_type;
    int data_type;
    int data_len;
    char* data;
    long stamp; //message stamp
}message;

message* message_create(); //memory allocation of a new message
void message_destroy(message *msg); //memory destroy of a message
void message_flush(message *msg);//flush message's data
void message_copy(message *msg_dst, message *msg_src);

//message translation
int bytes2message(buffer_ring_byte* bytes, message* msg); //return the length of the message
int message2bytes(message* msg, char** bytes); //return the length of the byte

//message properties
int message_isreq(message* msg); //check if message is req messgae
int message_tx_dest(message* msg); //get tx message destination 

//fifo message queue
//////////////////////////////////////////
#define MSG_Q_MAX_LEN  100 //max length of a message queue

typedef struct message_queue{
    message* msg;
    struct message_queue* prev;
    struct message_queue* next;
    struct timeval time;
}message_queue;

message_queue* message_queue_create(); //create a message queue item
message_queue* message_queue_flush(message_queue* msg_q); //flush the queue
void message_queue_destroy(message_queue* msg_q); //destroy the whole queue

void message_queue_init(message_queue* msg_q); //initiate an empty message queue
message_queue* message_queue_put(message_queue* msg_q, message* msg); //put one message to the tail fo the queue
message_queue* message_queue_get(message_queue* msg_q, message* msg); //get one message from the head of the queue 

message_queue* message_queue_to_head(message_queue* msg_q); //move pointer to the head
message_queue* message_queue_to_tail(message_queue* msg_q); //move pointer to the tail
int message_queue_getlen(message_queue* msg_q); //get the length of queue

//operations for req message queue
////////////////////////////////////////////////////
int message_queue_del(message_queue** msg_q);//delete a message queue item at current position msg_q, return to the current position of the msg_q 
int message_queue_del_stamp(message_queue** msg_q, long stamp); //delete all messages with stamp in the queue, return to number of deletion
int message_queue_find_stamp(message_queue* msg_q, long stamp); //find if stamp is in the queue, return to number of finding 

//message create
////////////////////////////////////////////////////
message* message_create_stat(int stat_len, char* stat, char id_gw[8], char id_dev[8], long stamp);
message* message_create_ctrl(int ctrl_len, char* ctrl, char id_gw[8], char id_dev[8], long stamp);
message* message_create_sync(int stat_len, char* stat, long u_stamp, char id_gw[8], char id_dev[8], long stamp);
message* message_create_req_auth_gw(int lic_len, char* lic, char id_gw[8], long stamp);
message* message_create_req_auth_dev(char id_gw[8], char id_dev[8], long stamp);
message* message_create_req_user(char id_gw[8], char id_user[8], long stamp);
message* message_create_pulse(char id_gw[8], long stamp);
message* message_create_req_stamp(char id_gw[8], long stamp);

#endif
