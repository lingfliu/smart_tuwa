#ifndef _PROC_
#define _PROC_


//Code list

//device type
#define DEV_TYPE_SWITCH_SINGLE 1
#define DEV_TYPE_SWITCH_DOUBLE 2
#define DEV_TYPE_SWITCH_TRIPLE 3
#define DEV_TYPE_SWITCH_DIMMABLE 4
#define DEV_TYPE_CURTAIN_SINGLE 5
#define DEV_TYPE_MECH 6

#define DEV_TYPE_SENSOR_SMOKE 101
#define DEV_TYPE_SENSOR_CO 102
#define DEV_TYPE_SENSOR_WATER 103
#define DEV_TYPE_SENSOR_INFRA 104
#define DEV_TYPE_SENSOR_TEMP 105
#define DEV_TYPE_SENSOR_HUMI 106
#define DEV_TYPE_SENSOR_TEMPHUMI 107


#define DEV_TYPE_INFRACTRL 201

//data type
#define DATA_TYPE_STAT 1 //status update
#define DATA_TYPE_CTRL 2 //control
#define DATA_TYPE_SYS_STAT_RST 101 //status reset/init
#define DATA_TYPE_SYS_NET_HB 201 //heart beat to server	

//data contents
#define DATA_STAT_ON  0xFF
#define DATA_STAT_OFF 0x00
//#define DATA_STAT_LEVEL 
#define DATA_STAT_ERR 0xAA

#define DATA_CTRL_ON 0xFF
#define DATA_CTRL_OFF 0x00
//#define DATA_CTRL_LEVEL 

#define DATA_NET_HB  0xAA
#define DATA_NET_ACK 0x00

typedef struct{
    char[2] header;
    int[9] time;
    char[6] gateway_id;
    char[6] dev_id;
    int dev_type;
    int data_type;
    int data_len;
    char* data;
}message;

typedef struct{
    int year;
    int month;
    int hour;
    int minute;
    int second;
}struct_time;


typedef struct{
    char[6] id;
    int znode_num;
    struct_znode* znode_list;
    struct_time time_last;
}struct_gateway;

typedef struct{
    char[6] id;
    char[6] id_parent;
    struct_time time_last;
}

message str2msg(char* data);
char* msg2str(code cmd);
int compare_time(struct_time time1, struct_time time2);

#endif
