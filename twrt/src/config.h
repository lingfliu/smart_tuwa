#ifndef _CONFIG_
#define _CONFIG_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_ex.h"
#include "simple_serial.h"
#include "simple_inet.h"

#define FILE_CFG "/etc/twrt/config"//config file
#define FILE_CFG_PARSER '='// config parser

#define FILE_LIC "/root/lic" //GW license file
#define FILE_ID_SYS "/root/id_sys" //ID (Wifi MAC) of GW 
#define FILE_INSTALL "/root/install" //status file name 
#define FILE_PASSWORD "/root/password" //place to store password
#define FILE_SCENE "/root/scene" //scene file 

/*
 * test code
 */
//#define FILE_ID_SYS "/etc/twrt/id_sys"
//#define FILE_LIC "/home/liulingfeng/lic" //GW license file
//#define FILE_PASSWORD "/home/liulingfeng/password" //place to store password
#define DEFAULT_LIC "0000000000000000"

//serial, server, and localhost configuration
#define CFG_SERIAL_NAME "SERIAL_NAME"
#define CFG_SERIAL_TYPE "SERIAL_TYPE"
#define CFG_SERIAL_BAUDRATE "SERIAL_BAUDRATE"
#define CFG_SERVER_IP "SERVER_IP"
#define CFG_SERVER_PORT "SERVER_PORT"
#define CFG_SERVER_PROC "SERVER_PROC"
#define CFG_LOCALHOST_PORT "LOCALHOST_PORT"
#define CFG_LOCALHOST_PROC "LOCALHOST_PROC"

//regular update of sys.znode
#define CFG_TIME_ZNET_SYNC "TIME_ZNET_SYNC"

//buffer length for config file getl
#define CFG_BUFF_LEN 100 

typedef struct{
    //serial config
    char serial_name[20];
    int serial_type;
    char serial_baudrate[20]; 

    //server config
    char server_ip[20];
    int server_port;
    int server_proc;   

	//local host config
    int localhost_port;
    int localhost_proc;

	//system config
	int time_znet_sync; // znet synchronization cycle in seconds

}config;

void get_config(config* cfg);
void parse_config(config* cfg, char* str);

#endif
