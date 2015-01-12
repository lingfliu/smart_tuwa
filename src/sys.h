#ifndef _SYS_
#define _SYS_

#define MAX_NUM_DEV_ZIGBEE 200
#define SYS_SUCCESS 1
#define SYS_FAIL 2

typedef struct{
    int[4] id_gateway;
    int dev_num;
    time last_update;
    device* dev_list;
}sys_stat;

typedef struct{
    int type;
    int[6] id;
    int[6] id_parent;
    char* stat;
}device;

typedef struct{
    int year;
    int month;
    int hour;
    int minute;
    int second;
    int index;
}time;

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
