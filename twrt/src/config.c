#include "config.h"

#define FILE_CONFIG_PARSER '='// config parser

void get_config(struct_config* cfg){
    FILE* fp;
    char str[CFG_FGETS_BUFF_LEN];
    fp = fopen(FILE_CONFIG,"r");
    if(fp == NULL){
	perror("Cannot access config file, program abort.\n");
	return;
    }
    while(fgets(str,CFG_FGETS_BUFF_LEN,fp)!=NULL)
	parse_config(cfg,str);
    fclose(fp);
}

void parse_config(struct_config* cfg, char* str){
    //remove blank lines
    int n;
    int idx_parser;
    int str_len = strlen(str);
    char cfg_item_name[CFG_FGETS_BUFF_LEN]="";
    char cfg_item_val[CFG_FGETS_BUFF_LEN]="";

    while(str[n] == '\n' || str[n] == '\t')
	n++;
    if(str[n] == '\n')
	return;
    str +=n;
    idx_parser = -1;
    for(n = 0; n<str_len; n++)
	if(str[n] == FILE_CONFIG_PARSER){
	    idx_parser = n;
	    break;
	}
    if(idx_parser == -1 || idx_parser == str_len || idx_parser == 0)
	return;
    strsub(str, cfg_item_name, 0, idx_parser-1);
    strsub(str, cfg_item_val, idx_parser+1, str_len-1);
    strtrim(cfg_item_name);
    strtrim(cfg_item_val);

    //identify the configs
    if(!strcmp(cfg_item_name, CFG_ITEM_SERIAL_NAME))
	strcpy(cfg->serial_name, cfg_item_val);
    if(!strcmp(cfg_item_name, CFG_ITEM_SERIAL_TYPE))
	sscanf(cfg_item_val,"%d",&(cfg->serial_type));
    if(!strcmp(cfg_item_name, CFG_ITEM_SERIAL_BAUDRATE))
	sscanf(cfg_item_val,"%d",&(cfg->serial_baudrate));
    if(!strcmp(cfg_item_name, CFG_ITEM_INET_SERVER_IP))
	strcpy(cfg->inet_server_ip, cfg_item_val);
    if(!strcmp(cfg_item_name, CFG_ITEM_INET_SERVER_PORT))
	sscanf(cfg_item_val,"%d",&(cfg->inet_server_port));
    if(!strcmp(cfg_item_name, CFG_ITEM_INET_SERVER_PROC))
	sscanf(cfg_item_val,"%d",&(cfg->inet_server_proc));
    if(!strcmp(cfg_item_name, CFG_ITEM_INET_CLIENT_PORT))
	sscanf(cfg_item_val,"%d",&(cfg->inet_client_port));
    if(!strcmp(cfg_item_name, CFG_ITEM_INET_CLIENT_PROC))
	sscanf(cfg_item_val,"%d",&(cfg->inet_client_proc));
    if(!strcmp(cfg_item_name, CFG_ITEM_FILE_STAT))
	strcpy(cfg->file_stat, cfg_item_val);
    if(!strcmp(cfg_item_name, CFG_ITEM_FILE_LOG))
	strcpy(cfg->file_log, cfg_item_val);
}
