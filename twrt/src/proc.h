#ifndef _PROC_
#define _PROC_

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>
#include "buffer_ex.h"

/************************************************
  A message's format: 
  HEADER MSG_STAMP ID_GW ID_DEV TYPE_DEV TYPE_DATA LEN_DATA DATA
  N.B. MSG_STAMP is not used
  **********************************************/

/************************************************
  device type
  ***********************************************/
#define DEV_SWITCH_1 1
#define DEV_SWITCH_2 2
#define DEV_SWITCH_3 3
#define DEV_SWITCH_4 4
#define DEV_DIMSWITCH_1 5
#define DEV_CURTAIN_1 6
#define DEV_CURTAIN_2 7
#define DEV_SOCK_1 8
#define DEV_SOCK_4 9

/*************************
 newly added device types
 *************************/

#define DEV_LOCAL_PHONE 50 //local phone (app)
#define DEV_GW_FAN 51 //local GW fan 

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
#define DEV_DOUBLE_CTRL 204

#define NULL_DEV "00000000" 
/*************************************************
  Data type
  ************************************************

   Data formats: 
	1. Status and controls:
	For switches, sockets, curtain, and theme setter ctrl: CHN1_STAT CHN2_STAT CHN3_STAT CHN4_STAT
	For dimmables: CHN1_LEVEL CHN2_LEVEL CHN3_LEVEL
	For on/off sensors: STAT (ON/OFF)
	For value sensors: Value in byte format
	For mech_valve and alarm (control only): STAT (ON/OFF)
	
	2. Synchronization:
	System synchronization: sys_stamp (4 bytes) status
	Node synchronization: node_stamp (4 bytes) status

	3. Stamp request: (during initialization)

	4. Pulse: NULL_DATA (1 byte)

	5. GW authorization: lic

	6. Theme setter: DEV_ID NUM ZNET_STATUS

	7. Device install (for the znet to merge the data): DESCRIPTION_LENGTH(2 bytes) DESCRIPTION (varying)
   **********************************************/

#define DATA_STAT 1 //status update, from znode to gateway
#define DATA_CTRL 2 //control, from znode to gateway, from gateway to znode, from server to gateway
#define DATA_SET 3 //data type specially for theme setter

#define DATA_PULSE 4 //simple pulse msg for sockets (both to server and from localhost) 
#define DATA_SYNC 5 //simple sync msg for localusers

///////////////////////////////////////////////////////////////
//from app and server to GW
#define DATA_SET_INSTALL 6 //set installation of a device 
#define DATA_GET_INSTALL 7 //request device installation information

#define DATA_INSTALL 8 //device installation data for data_get_install request (to znet)
#define DATA_INSTALL_INFO 9 //not used 

#define DATA_DEL_INSTALL 10 //delete device installation data from the system

#define DATA_FINISH_INSTALL 11 //finishing the installation, call the GW to save the data to file

#define DATA_DEL_ZNODE 12 //delete znode from znet

#define DATA_SCENE_CTRL 13 //scene command from app
#define DATA_GET_SCENE 14 //get scene
#define DATA_SET_SCENE 15 //set a scene
#define DATA_SCENE 16 //scene data for get_scene request
#define DATA_FINISH_SCENE 17 //finish scene setting and save the data to file
#define DATA_DELETE_SCENE 18 //delete a scene

///////////////////////////////////////////////////////////////

#define DATA_REQ_SYNC 21 //synchronization request
#define DATA_REQ_AUTH_GW 22 //gw authroization request
#define DATA_REQ_AUTH_DEV 23 //dev authroization request (not used)
#define DATA_REQ_USER 24 //user list request (not used)
#define DATA_REQ_STAMP 25 //initial stamp request
#define DATA_REQ_PULSE 26 //tcp pulse (not req any longer)

#define DATA_REQ_STAT 27 //localuser request for znet status
#define DATA_REQ_AUTH_LOCAL 28 //localuser auth request

#define DATA_ACK_SYNC 61 //synchronization ack (not used)
#define DATA_ACK_AUTH_GW 62 //auth gw ack
//#define DATA_ACK_AUTH_DEV 63 //auth dev ack (not used)
#define DATA_ACK_USER 64 //user list ack (not used)
#define DATA_ACK_STAMP 65 //initial stamp ack 
//#define DATA_ACK_PULSE 66 //tcp pulse ack (not used)

#define DATA_ACK_AUTH_LOCAL 67 //localuser auth ack

/*new messages for scene operation*/
#define DATA_ACK_INSTALL_OP 69
#define DATA_ACK_SCENE_OP 70

#define DATA_SYS_RESET 101 //sys reset command (not used)

/*
 * password management
 */
#define DATA_SET_PASSWORD 102 //set local user login password
#define DATA_SET_LIC 103 //set server login password
#define DATA_SET_PASSWORD_ACK 104
#define DATA_SET_LIC_ACK 105

#define DATA_NULL 201  //null data type for general purposes

/*************************************************
  Data contents
  ***********************************************/
#define STAT_ON  100 
#define STAT_OFF 0 
//tunable stat range from 0 to 100 in 1 byte

#define ERR_NET 0xF0 // (not used)
#define ERR_ELEC 0xF1 //(not used)
#define ERR_SYS 0xF2 //(not used)

#define TCP_PULSE 0xAA

#define AUTH_OK 0
#define AUTH_NO 0xFF

#define CTRL_ON 100
#define CTRL_OFF 0 
//level control range from 0~100 

#define THEME_SET 0x11
//theme set, when this data is send to GW, GW should record the current system status: dev_id+num+sys_status

#define BYTE_NULL 0x00 //(not used)

//message header
#define MSG_HEADER_GW "AADD" //for packets over gw server and znet, 4 bytes
#define MSG_HEADER_FE "A" //for packets over znode and fe, 1 byte

/*************************************************
  Message properties
  minimal message structure: 
  header stamp id_gw id_dev dev_type data_type data_len data
  4+     4+    8+    8+     2+       2+        2+       x = 30+x
  ***********************************************/
#define MSG_LEN_MIN 30//minimum length of a message
#define MSG_LEN_HEADER_GW 4 
#define MSG_LEN_HEADER_FE 1 
#define MSG_LEN_STAMP 4 
#define MSG_LEN_ID_GW 8 
#define MSG_LEN_ID_DEV 8 
#define MSG_LEN_DEV_TYPE 2 
#define MSG_LEN_DATA_TYPE 2 
#define MSG_LEN_DATA_LEN 2 

#define MSG_LEN_FIXED 30  //fixed length for all types of messages

#define MSG_POS_STAMP 4
#define MSG_POS_ID_GW 8
#define MSG_POS_ID_DEV 16
#define MSG_POS_DEV_TYPE 24
#define MSG_POS_DATA_TYPE 26
#define MSG_POS_DATA_LEN 28

//message destination
#define MSG_TO_ZNET 1
#define MSG_TO_SERVER 2
#define MSG_TO_LOCALUSER 3

//system message parameters
#define SYS_LEN_STATUS 50 //status length of gw
#define SYS_LEN_LIC 16  //lic string length
#define SYS_LEN_AUTHCODE 8//cookie length
#define DEFAULT_VER 1 //default version of device
#define DEFAULT_MODEL 1 //default vendor of all devices

/***********************************************
   message
  **********************************************/
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
void message_copy(message *msg_dst, message *msg_src); //copy one message to another message

//message translation
int bytes2message(buffer_ring_byte* bytes, message* msg); //return the length of the message
int message2bytes(message* msg, char* bytes); //return the length of the byte

//message properties
int message_isreq(message* msg); //check if message is req messgae
int message_tx_dest(message* msg); //get tx message destination 
int message_isvalid(message* msg);

//fifo message queue
//////////////////////////////////////////
#define MSG_Q_MAX_LEN  100 //max length of a message queue

typedef struct message_queue{
    message msg;
    struct message_queue* prev;
    struct message_queue* next;
    struct timeval time;
}message_queue;

message_queue* message_queue_create(); //create a message queue item
message_queue* message_queue_flush(message_queue* msg_q); //flush the queue
void message_queue_destroy(message_queue* msg_q); //destroy the whole queue

void message_queue_init(message_queue* msg_q); //initiate an empty message queue's head
message_queue* message_queue_put(message_queue* msg_q, message* msg); //put one message to the tail fo the queue
message_queue* message_queue_get(message_queue* msg_q, message* msg); //get one message from the head of the queue 

message_queue* message_queue_to_head(message_queue* msg_q); //move pointer to the head
message_queue* message_queue_to_tail(message_queue* msg_q); //move pointer to the tail
int message_queue_getlen(message_queue* msg_q); //get the length of queue

int message_queue_del(message_queue** msg_q);//delete a message queue item at current position msg_q, return to the current position of the msg_q 
int message_queue_del_stamp(message_queue** msg_q, long stamp); //delete all messages with stamp in the queue, return to number of deletion
int message_queue_find_stamp(message_queue* msg_q, long stamp); //find if stamp is in the queue, return to number of finding 

/************************************************
  message creation
 ************************************************/
message* message_create_stat(int stat_len, char* stat, char id_gw[8], char id_dev[8], int dev_type);
message* message_create_ctrl(int ctrl_len, char* ctrl, char id_gw[8], char id_dev[8], int dev_type);
message* message_create_sync(int stat_len, char* stat, long u_stamp, char id_gw[8], char id_dev[8], int dev_type);
message* message_create_req_auth_gw(int lic_len, char* lic, char id_gw[8], long stamp);
message* message_create_req_auth_dev(char id_gw[8], char id_dev[8], long stamp);
message* message_create_req_user(char id_gw[8], char id_user[8], long stamp);
message* message_create_pulse(char id_gw[8]);
message* message_create_req_stamp(char id_gw[8], long stamp);
message* message_create_null(char id_gw[8], long stamp);
message* message_create_ack_auth_local(char id_gw[8], char id_dev[8], int dev_type, char auth_result);

message* message_create_install(char id_gw[8], char id_dev[8], int type); //create install info and send to znet
//message* message_create_del_install(char id_gw[8], char id_dev[8]); //delete install into and send to znet
message* message_create_install_info(char id_gw[8], char id_dev[8], int dev_type, int len_descrip, char* descrip); //create install info and send to znet

message* message_create_set_password_ack(char id_gw[8]);
message* message_create_set_lic_ack(char id_gw[8]);
message* message_create_del_znode(char id_gw[8], char id_dev[8]);

//message* message_create_req_stat(char id_dev[8]);
message* message_create_ack_install_op(char id_gw[8], char id_dev[8], int op_code, int result);
message* message_create_ack_scene_op(char id_gw[8], char id_major[8], char id_minor[8], int op_code, int result);
#endif
