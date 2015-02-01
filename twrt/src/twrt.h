#ifndef _TWRT_
#define _TWRT_


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>

#include "simple_serial.h"
#include "simple_inet.h"
#include "proc.h"
#include "sys.h"
#include "config.h"

//defines 
#define BUFF_RW_LEN 500
#define BUFF_RING_LEN 1000
#define MSG_Q_LEN 50 

//authentication variables

//configs and entities
static struct_config cfg;
static struct_serial serial;
static struct_inet client;

//threads
static pthread_t thrd_serial_rx;
static pthread_t thrd_serial_tx;
static pthread_t thrd_client_rx;
static pthread_t thrd_client_tx;

static pthread_t thrd_trans_serial;
static pthread_t thrd_trans_client;

static pthread_t thrd_sys_msg_handler;

static pthread_t thrd_sys_ptask; //thread for performing periodic sys task

//mut and cond
static pthread_mutex_t mut_serial;
static pthread_cond_t cond_serial;
static pthread_mutex_t mut_client;
static pthread_cond_t cond_client;
static pthread_mutex_t mut_msg_handler;
static pthread_cond_t cond_msg_handler;

//runnables
void *run_serial_rx();
void *run_inet_client_rx();
void *run_translate_serial();
void *run_translate_inet_client();
void *run_sys_msg_handler();
void *run_sys_ptask();

//buffers
buffer_byte_ring *buff_serial;
buffer_byte_ring *buff_client;
char read_serial[BUFF_RW_LEN];
char write_serial[BUFF_RW_LEN];
char read_client[BUFF_RW_LEN];
char write_client[BUFF_RW_LEN];

//message queue
static struct_message *msg_serial;
static struct_message *msg_client;
static struct_message_queue* msg_queue;
static struct_message_queue* msg_queue_req;
static long msg_stamp;

//functions
int handle_msg(struct_message *msg);

//timer for the sys_ptask
static time_t time_net_hb;
static time_t time_sys_reset;
#endif
