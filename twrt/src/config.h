#ifndef _CONFIG_
#define _CONFIG_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_ex.h"
#include "simple_serial.h"
#include "simple_inet.h"

#define FILE_CONFIG "/etc/smart_tuwa/config"//config file
#define FILE_CONFIG_PARSER '='// config parser

#define CFG_ITEM_SERIAL_NAME "SERIAL_NAME"
#define CFG_ITEM_SERIAL_TYPE "SERIAL_TYPE"
#define CFG_ITEM_SERIAL_BAUDRATE "SERIAL_BAUDRATE"
#define CFG_ITEM_INET_SERVER_IP "INET_SERVER_IP"
#define CFG_ITEM_INET_SERVER_PORT "INET_SERVER_PORT"
#define CFG_ITEM_INET_SERVER_PROC "INET_SERVER_PROC"
#define CFG_ITEM_INET_CLIENT_PORT "INET_CLIENT_PORT"
#define CFG_ITEM_INET_CLIENT_PROC "INET_CLIENT_PROC"

#define CFG_ITEM_FILE_STAT "FILE_STAT"
#define CFG_ITEM_FILE_LOG "FILE_LOG"

#define CFG_FGETS_BUFF_LEN 100
typedef struct{
    //serial config
    char serial_name[50];
    int serial_type;
    int serial_baudrate; 

    //inet config
    char inet_server_ip[50];
    int inet_server_port;
    int inet_server_proc;   
    int inet_client_port;
    int inet_client_proc;

    //application config
    char file_stat[50];
    char file_log[50];
}struct_config;

void get_config(struct_config* cfg);
void parse_config(struct_config* cfg, char* str);
#endif
