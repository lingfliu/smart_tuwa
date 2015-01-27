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

int sys_reset(struct_sys_status* sys_status);
int sys_save_sys_status(char* file_name);
int sys_load_sys_status(char* file_name);
int sys_clock_sync(struct_config* cfg);

#endif
