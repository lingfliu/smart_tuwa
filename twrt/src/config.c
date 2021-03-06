#include "config.h"

void get_config(config* cfg){
	FILE* fp;
	char str[CFG_BUFF_LEN];
	fp = fopen(FILE_CFG,"r");
	if(fp == NULL){
		//perror("Cannot read config file, program abort.\n");
		return;
	}

	while(fgets(str,CFG_BUFF_LEN,fp)!=NULL) {
		parse_config(cfg,str);
	}

	fclose(fp);
}

void parse_config(config* cfg, char* str){
	//remove blank lines
	int n = 0;
	int idx_parser;
	int str_len = strlen(str);
	char cfg_name[CFG_BUFF_LEN]="";
	char cfg_val[CFG_BUFF_LEN]="";

	while(str[n] == ' ' || str[n] == '\t') {//if white space at head
		n++;
	}

	if(str[n] == '\n') {//if empty lines
		return;
	}

	str +=n;
	idx_parser = -1;
	for(n = 0; n<str_len; n++) {
		if(str[n] == FILE_CFG_PARSER) {
			idx_parser = n;
			break;
		}
	}
	if(idx_parser == -1 || idx_parser == str_len || idx_parser == 0) {
		return;
	}
	strsub(str, cfg_name, 0, idx_parser-1);
	strsub(str, cfg_val, idx_parser+1, str_len-1);
	strtrim(cfg_name);
	strtrim(cfg_val);

	//identify the configs
	if(!strcmp(cfg_name, CFG_SERIAL_NAME))
		strcpy(cfg->serial_name, cfg_val);
	if(!strcmp(cfg_name, CFG_SERIAL_TYPE))
		sscanf(cfg_val,"%d",&(cfg->serial_type));
	if(!strcmp(cfg_name, CFG_SERIAL_BAUDRATE))
		strcpy(cfg->serial_baudrate, cfg_val);
	if(!strcmp(cfg_name, CFG_SERVER_IP))
		strcpy(cfg->server_ip, cfg_val);
	if(!strcmp(cfg_name, CFG_SERVER_PORT))
		sscanf(cfg_val,"%d",&(cfg->server_port));
	if(!strcmp(cfg_name, CFG_SERVER_PROC))
		sscanf(cfg_val,"%d",&(cfg->server_proc));
	if(!strcmp(cfg_name, CFG_LOCALHOST_PORT))
		sscanf(cfg_val,"%d",&(cfg->localhost_port));
	if(!strcmp(cfg_name, CFG_LOCALHOST_PROC))
		sscanf(cfg_val,"%d",&(cfg->localhost_proc));
	if(!strcmp(cfg_name, CFG_TIME_ZNET_SYNC))
		sscanf(cfg_val,"%d",&(cfg->time_znet_sync));
}
