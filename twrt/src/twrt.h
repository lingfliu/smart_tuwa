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
#define BUFF_RW_LEN 255
#define BUFF_RING_LEN 1000
#define MSG_QUEUE_LEN 50 

//configs and entities
static struct_config cfg;
static struct_serial serial;
static struct_inet inet_client;

//threads
static pthread_t thrd_serial_rx;
static pthread_t thrd_serial_tx;
static pthread_t thrd_inet_client_rx;
static pthread_t thrd_inet_client_tx;

static pthread_t thrd_translate_serial;
static pthread_t thrd_translate_inet_client;

static pthread_t thrd_sys;

//mut and cond
static pthread_mutex_t mut_serial_buff;
static pthread_mutex_t mut_inet_client_buff;
static pthread_cond_t cond_serial_buff;
static pthread_cond_t cond_inet_client_buff;

//runnables
void *run_serial_rx();
void *run_inet_client_rx();
void *run_translate_serial();
void *run_translate_inet_client();
void *run_sys();

//buffers
buffer_byte_ring buff_serial;
buffer_byte_ring buff_inet_client;
char read_serial[BUFF_RW_LEN];
char write_serial[BUFF_RW_LEN];
char read_inet_client[BUFF_RW_LEN];
char write_inet_client[BUFF_RW_LEN];

//packet queue
static struct_message_queue* msg_queue;
static len_msg_queue;
#endif
