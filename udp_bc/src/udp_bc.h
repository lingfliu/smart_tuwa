#ifndef _UDP_BC_
#define _UDP_BC_


#include "utils.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


#define BC_PORT 9005
#define BC_ADDR "255.255.255.255"

static struct timeval inet_timeout;
static struct timeval timer;

static int sock;

/***********************************************
   result for multiple uses
   *********************************************/
static int retval;

#endif
