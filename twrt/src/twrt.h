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
#define BUFF_RW_LEN 500 //read & write length of io 
#define BUFF_RING_LEN 1000 // read & write length of byte buffer
#define MSG_Q_LEN 50  //length of message queue

#define TIMER_PULSE 1000 //pulse ack waiting time in milli-second
#define TIMER_RESET 100 //pulse ack waiting time in hour 
#define TIMER_REQ 2000 //request waiting time in milli-second
#define TIMER_SYNC 30000 //synchronization time in milli-second

#define INET_STAT_ON 1 //remote inet (client) stat
#define INET_STAT_OFF 0 //remote inet (client) stat

//configs and io entities
static config cfg;
static serial srl;
static inet client;

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
void *run_serial_tx();
void *run_client_rx();
void *run_client_tx();
void *run_trans_serial();
void *run_trans_client();
void *run_sys_msg_rx();
void *run_sys_msg_tx();
void *run_sys_ptask();

//buffers
static buffer_ring_byte buff_serial; //buffer for serial reading
static buffer_ring_byte buff_client; //buffer for inet client reading
char read_serial[BUFF_RW_LEN];  //buffer for serial read & write io
char read_client[BUFF_RW_LEN]; //buffer for client read & write io

//message queues
static message_queue* msg_q_rx;  //message queue tail for saving rx msg
static message_queue* msg_q_rx_h; //head
static message_queue* msg_q_tx; //message queue tail for saving tx msg
static message_queue* msg_q_tx_h; //head
static message_queue* msg_q_tx_req; //message queue tail for saving send req msg
static message_queue* msg_q_tx_req_h; //head

//sys_t
static sys_t sys;

//inet flags
static int inet_stat;

//functions
void on_inet_client_disconnect(); //handling when disconnected from the server 
int handle_msg_rx(message *msg); //rx msg handling
void on_req_failed(struct_message *msg); //req msg failed handling

//time functions
int timediff(struct timeval time1, struct timeval time2);
int timediff_hour(struct timeval time1, struct timeval time2);

#endif
