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
#define FILE_LIC "/etc/twrt/lic"
#define FILE_ID_SYS "/etc/twrt/id_sys"
#define FILE_STAT "/etc/twrt/stat_sys"
#define FILE_LOG "/etc/twrt/log"

#define CFG_SERIAL_NAME "SERIAL_NAME"
#define CFG_SERIAL_TYPE "SERIAL_TYPE"
#define CFG_SERIAL_BAUDRATE "SERIAL_BAUDRATE"
#define CFG_SERVER_IP "SERVER_IP"
#define CFG_SERVER_PORT "SERVER_PORT"
#define CFG_SERVER_PROC "SERVER_PROC"
#define CFG_HOST_PORT "HOST_PORT"
#define CFG_HOST_PROC "HOST_PROC"

#define CFG_TIME_UPDATE "TIME_UPDATE"

#define CFG_BUFF_LEN 50 

typedef struct{
    //serial config
    char serial_name[20];
    int serial_type;
    char serial_baudrate[20]; 

    //inet config
    char server_ip[20];
    int server_port;
    int server_proc;   
    int host_port;
    int host_proc;

	//system config
	int time_update; // system update period in minutes

}config;

void get_config(config* cfg);
void parse_config(config* cfg, char* str);

#endif
