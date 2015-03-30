#include "config.h"

void get_config(config* cfg){
    FILE* fp;
    char str[CFG_BUFF_LEN];
    fp = fopen(FILE_CFG,"r");
    if(fp == NULL){
	perror("Cannot read config file, program abort.\n");
	return;
    }
    while(fgets(str,CFG_BUFF_LEN,fp)!=NULL)
	parse_config(cfg,str);
    //check the unset paras
    fclose(fp);
}

void parse_config(config* cfg, char* str){
    //remove blank lines
    int n;
    int idx_parser;
    int str_len = strlen(str);
    char cfg_item_name[CFG_BUFF_LEN]="";
    char cfg_item_val[CFG_BUFF_LEN]="";

    while(str[n] == ' ' || str[n] == '\t')//if white space at head
	n++;
    if(str[n] == '\n')//if empty lines
	return;
    str +=n;
    idx_parser = -1;
    for(n = 0; n<str_len; n++)
	if(str[n] == FILE_CFG_PARSER){
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
	strcpy(cfg->serial_baudrate, cfg_item_val);
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
}
