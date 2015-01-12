#ifndef _CONFIG_
#define _CONFIG_


#define FILE_CONFIG "/etc/smart_tuwa/config"//config file
#define FILE_CONFIG_PARSER '='// config parser

typedef struct{
    //serial config
    char serial_name[50];
    int serial_type;
    int serial_baudrate; 

    //inet config
    char server_ip[50];
    int server_port;
    int server_type;   
    int client_port;
    int client_type;

    //application config
    char file_stat[50];
    char file_log[50];
}struct_config;

struct_config get_config();
void parse_config();
#endif
