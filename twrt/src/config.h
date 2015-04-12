#ifndef _CONFIG_
#define _CONFIG_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_ex.h"
#include "simple_serial.h"
#include "simple_inet.h"

#define FILE_CFG "/etc/smart_tuwa/config"//config file
#define FILE_CFG_PARSER '='// config parser
#define FILE_LIC "/etc/smart_tuwa/lic"
#define FILE_ID_SYS "/etc/smart_tuwa/id_sys"
#define FILE_STAT "/etc/smart_tuwa/stat_sys"
#define FILE_LOG "/etc/smart_tuwa/log"

#define CFG_ITEM_SERIAL_NAME "SERIAL_NAME"
#define CFG_ITEM_SERIAL_TYPE "SERIAL_TYPE"
#define CFG_ITEM_SERIAL_BAUDRATE "SERIAL_BAUDRATE"
#define CFG_ITEM_INET_SERVER_IP "INET_SERVER_IP"
#define CFG_ITEM_INET_SERVER_PORT "INET_SERVER_PORT"
#define CFG_ITEM_INET_SERVER_PROC "INET_SERVER_PROC"
#define CFG_ITEM_INET_CLIENT_PORT "INET_CLIENT_PORT"
#define CFG_ITEM_INET_CLIENT_PROC "INET_CLIENT_PROC"

#define CFG_BUFF_LEN 100 
typedef struct{
    //serial config
    char serial_name[20];
    int serial_type;
    char serial_baudrate[20]; 

    //inet config
    char inet_server_ip[20];
    int inet_server_port;
    int inet_server_proc;   
    int inet_client_port;
    int inet_client_proc;

}config;

void get_config(config* cfg);
void parse_config(config* cfg, char* str);

#endif
