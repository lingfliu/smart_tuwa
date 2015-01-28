#ifndef _SYS_
#define _SYS_

#include "config.h"
#include "proc.h"
#include "simple_inet.h"
#include "simple_serial.h"

#include <string.h>

#define SYS_SUCCESS 1
#define SYS_FAIL 2


//configuration
int serial_config(struct_config* cfg, struct_serial* serial);
int inet_client_config(struct_config* cfg, struct_serial* inet_client);
int inet_server_config(struct_config* cfg, struct_serial* inet_server);

//system initialization
int sys_init(struct_sys* sys);//allocate memory space for sys
int sys_reset(struct_sys* sys); //reset the system
void sys_flush(struct_sys* sys); //flush the system for reset

//znode operation 
int znode_copy(struct_znode* znode_dst, struct_znode* znode_src);
void znode_flush(struct_znode* znode);
int sys_znode_get(struct_sys* sys, char[6] znode_id, struct_znode* znode);
int sys_znode_get_max_stamp(struct_sys* sys);
int sys_znode_update(struct_sys* sys, struct_znode* znode);

//synchronization
int sys_sync(struct_sys* sys, struct_inet_client* inet_client);//synchronize the system znet status
int sys_clock_sync(struct_sys* sys, struct_config* cfg); //synchronize the system clock online

//network operation
int sys_net_hb(struct_inet_client* inet_client);
int sys_net_auth(struct_sys* sys, struct_inet_client* client);

//local save/load 
int sys_save_sys(char* file_name);
int sys_load_sys(char* file_name);
#endif
