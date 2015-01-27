#include "sys.h"

void serial_config(struct_config* cfg, struct_serial* serial){
    strcpy(serial->name, cfg->serial_name);
    serial->type = cfg->serial_type;
    strcpy(serial->baudrate, cfg->serial_baudrate);
}

int inet_client_config(struct_config* cfg, struct_serial* inet_client){
    inet_client->proc = cfg->inet_client_proc;

    switch(inet_client->proc){
	case INET_PROC_TCP:
	    if(inet_client->fp = socket(AF_INET, SOCK_STREAM, 0) < 0){
		perror("create socket failed\n");
		return -1;
	    }
	    break;
	case INET_PROC_UDP:
	    if(inet_client->fp = socket(AF_INET, SOCK_DGRAM, 0) < 0){
		perror("create socket failed\n");
		return -1;
	    }
	    break;
	default:
	    perror("Unknown proctocol\n");
	    return -1;
    }

    memset(inet_client->ip_addr, 0, sizeof(inet_client->ip_addr));
    inet_client->ip_addr.sin_family = AF_INET;
    inet_client->ip_addr.sin_port = htons(cfg->inet_client_port);
    if(inet_pton(AF_INET, cfg->inet_server_ip, inet_client->ip_addr.sin_addr)<= 0){
	perror("ip error\n");
	return -1;
    }
    return 0;
}

int inet_server_config(struct_config* cfg, struct_serial* inet_server){
    return 0;
}

int sys_reset(struct_sys_status* sys_status){

}

int sys_save_sys_status(char* file_name){
    
}

int sys_load_sys_status(char* file_name){

}

int sys_clock_sync(struct_config* cfg){

}
