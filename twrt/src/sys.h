#ifndef _SYS_
#define _SYS_

#include "proc.h"

#define MAX_NUM_DEV_ZIGBEE 200
#define SYS_SUCCESS 1
#define SYS_FAIL 2

sys_stat* sys_init();
sys_stat* sys_reset();
int sys_add_dev(device item);
int sys_remove_dev(device item);
int sys_update_dev(device item);
int sys_register_dev(device item);
int sys_unregister_dev(device item);
time sys_clock_sync();
int sys_save_sys_stat(char* file_name);
int sys_load_sys_stat(char* file_name);
int sys_lock();
#endif
