#ifndef _GW_CONTROL_
#define _GW_CONTROL_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "config.h"

#define ETC_NETWORK "/etc/config/network"
#define ETC_WIRELESS "/etc/config/wirelss"

void fan_control(int cmd);

//password management
int get_password(char* key);
int set_password(char* key_prev, char* key_new);
int get_lic(char* key);
int set_lic(char* key_prev, char* key_new);

void set_wds(char* ssid, int ssid_len, char* password, int password_len, char *encrypt, int encrypt_len);
void set_wifi(char* ssid, int ssid_len, char* password, int password_len, char *encrypt, int encrypt_len);
#endif

