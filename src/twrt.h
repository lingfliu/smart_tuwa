#ifndef _TWRT_
#define _TWRT_

#include "simple_serial.h"
#include "simple_inet.h"
#include "proc.h"
#include "sys.h"
#include "config.h"
#include <pthread.h>

static pthread_t thread_serial;
static pthread_t thread_inet;
static pthread_t thread_sys;

static struct_config config_twrt;
#endif
