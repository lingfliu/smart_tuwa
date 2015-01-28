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

int inet_server_config(struct_config* cfg, struct_inet* inet_server){
    return 0;
}

int sys_init(struct_sys* sys, long u_stamp, long g_stamp){
    sys = malloc(sizeof(sys));
    memset(sys, 0, sizeof(sys));
    int m;
    for(m = 0; m<PROC_MAX_ZNODE; m++)
	sys->znode_list[m]->u_stamp = u_stamp;
}

int sys_reset(struct_sys* sys){
    //bakup stamp
    long u_stamp = sys->u_stamp; 
    long g_stamp = sys->g_stamp;
    sys_flush(sys);
    //restore stamp
    sys->u_stamp = u_stamp;
    sys->g_stamp = g_stamp;
    return 0;
}

int sys_znode_update(struct_sys* sys, struct_znode* znode){
    int m;
    for(m = 0; m<PROC_MAX_ZNODE; m++)
	if(!memcmp(sys->znode_list[m].id, znode->id, 6)){
	    memcpy(sys->znode_list[m].status, znode->status, znode->status_len);
	    sys->znode_list[m].u_stamp = sys_get_max_stamp_znode(sys)+1;
	    break;
	}
	if(sys->znode_num<=PROC_MAX_ZNODE){//if still has space for new nodes
	    sys->znode_num++;
	    memcpy(sys->znode_list[sys->znode_num], znode, sizeof(znode));

	    sys->u_stamp++;
	}
}

int sys_get_max_stamp_znode(struct_sys* sys){
    int m;
    int max_stamp = 0;
    for(m = 0; m<PROC_MAX_ZNODE; m++)
	max_stamp = max_stamp>sys->znode_list[m]->u_stamp?max_stamp:sys->znode_list[m]->u_stamp;
    return max_stamp; 
}

void sys_stamp_sync(struct_sys* sys, long u_stamp, long g_stamp){
    int m;
    sys->u_stamp = u_stamp;
    for(m = 0; m<PROC_MAX_ZNODE; m++)
	sys->znode_list[m]->u_stamp =sys->znode_list[m]->u_stamp>u_stamp?sys->znode_list[m]->u_stamp:u_stamp;
}

int sys_save_sys(char* file_name){
    
}

int sys_load_sys(char* file_name){

}

int sys_clock_sync(struct_config* cfg){

}
