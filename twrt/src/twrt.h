#ifndef _TWRT_
#define _TWRT_

#include "simple_serial.h"
#include "simple_inet.h"
#include "proc.h"
#include "sys.h"
#include "config.h"
#include "utils.h"
#include "gw_control.h"


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
//configs and io entities
static config cfg;

/************************************************
  variables for serial
  ***********************************************/
static serial srl;

/************************************************
  variables for client socket connection
  ***********************************************/
static inet client;
static fd_set inet_fds;
static struct timeval inet_timeout;

/************************************************
   variables for localhost and localuser
   localhost don't perform reconnection of the socket
   when connection failed, localhost will close socket immediately
   **********************************************/
static inet localhost;
static pthread_t thrd_localhost;
void *run_localhost();
void *run_localuser_rx();
void *run_localuser_tx();

/***********************************************
   result for multiple uses
   *********************************************/
static int retval;
/***********************************************
   counter for bakup 
   *********************************************/
static int update_num;

//threads, runnables, muts and conds
static pthread_t thrd_serial_rx;
static pthread_t thrd_serial_tx;
static buffer_ring_byte buff_serial; //buffer for serial reading
char read_serial[BUFF_IO_LEN];  //buffer for serial read io
static pthread_mutex_t mut_serial; // lock for buffer_serial
static pthread_cond_t cond_serial;
void *run_serial_rx();
void *run_serial_tx();

static pthread_t thrd_client_rx;
static pthread_t thrd_client_tx;
static buffer_ring_byte buff_client; //buffer for inet client reading
char read_client[BUFF_IO_LEN]; //buffer for client read io
static pthread_mutex_t mut_client; // lock for buffer_client
static pthread_cond_t cond_client;
void *run_client_rx();
void *run_client_tx();

static pthread_t thrd_sys_msg_rx; //thread for reading incoming messages
static pthread_mutex_t mut_msg_rx; //lock for msg_q_rx
void *run_sys_msg_rx();

static pthread_t thrd_sys_msg_tx; //thread for sending scheduled messages
static pthread_mutex_t mut_msg_tx; //lock for msg_q_tx
void *run_sys_msg_tx();

static pthread_t thrd_sys_ptask; //thread for performing periodic sys task

static pthread_mutex_t mut_sys; //lock for the main thread, sys

//runnables
void *run_sys_ptask();

//message queues
static message_queue* msg_q_rx;  //message queue tail for saving rx msg
static message_queue* msg_q_rx_h; //head
static message_queue* msg_q_tx; //message queue tail for saving tx msg
static message_queue* msg_q_tx_h; //head
static message_queue* msg_q_tx_req; //message queue tail for saving send req msg
static message_queue* msg_q_tx_req_h; //head

//sys_t
static sys_t sys;

//functions
void on_inet_client_disconnect(); //handling when disconnected from the server 
int handle_msg_rx(message *msg); //rx msg handling
int handle_local_message(message *msg, localuser *usr); //message handling for localuser
#endif
