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

static pthread_t thrd_sys_msg_rx; //thread for reading incoming messages
static pthread_t thrd_sys_msg_tx; //thread for sending scheduled messages

static pthread_t thrd_sys_ptask; //thread for performing periodic sys task

//mut and cond
static pthread_mutex_t mut_serial; // lock for buffer_serial
static pthread_cond_t cond_serial;
static pthread_mutex_t mut_client; // lock for buffer_client
static pthread_cond_t cond_client;
static pthread_mutex_t mut_msg_rx; //lock for msg_q_rx
static pthread_cond_t cond_msg_rx;
static pthread_mutex_t mut_msg_tx; //lock for msg_q_tx
static pthread_cond_t cond_msg_tx;

//runnables
void *run_serial_rx();
void *run_client_rx();
void *run_trans_serial();
void *run_trans_client();
void *run_sys_msg_rx();
void *run_sys_msg_tx();
void *run_sys_ptask();

//buffers
buffer_byte_ring *buff_serial;
buffer_byte_ring *buff_client;
char read_serial[BUFF_RW_LEN];
char read_client[BUFF_RW_LEN];

//message queue
static struct_message_queue* msg_q_rx;
static struct_message_queue* msg_q_tx;

//functions
void on_inet_client_disconnect();
int handle_msg(struct_message *msg);
void on_req_failed(struct_message *msg);

//system
static struct_sys* sys;
//inet flags
static inet_stat;
#define TIME_INTERVAL_HB 1;
#define TIME_INTERVAL_REQ 1;
#endif
