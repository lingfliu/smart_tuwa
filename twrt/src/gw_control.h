#ifndef _GW_CONTROL_
#define _GW_CONTROL_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include "config.h"

void fan_control(int cmd);

//password management
int get_password(char* key);
int set_password(char* key_prev, char* key_new);
int get_lic(char* key);
int set_lic(char* key_prev, char* key_new);
#endif

