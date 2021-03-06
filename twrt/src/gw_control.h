#ifndef _GW_CONTROL_
#define _GW_CONTROL_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "config.h"
#include <string.h>

#define ETC_WIRELESS "/etc/config/wireless"
#define ETC_WIRELESS_AP "/etc/config/wireless_AP"
#define ETC_WIRELESS_APSTA "/etc/config/wireless_APSTA"

#define ETC_WIRELESS_COMMON "/root/wireless.common"
#define ETC_WIRELESS_COMPONENT_AP "/root/wireless.ap"
#define ETC_WIRELESS_COMPONENT_STA "/root/wireless.sta"

void fan_control(int cmd);

//password management
int get_password(char* key);
int set_password(char* key_prev, char* key_new);
int get_lic(char* key);
int set_lic(char* key_prev, char* key_new);


/*
 * new code for AP and STA wireless setting
 * */
int set_ap(char* ssid, int ssid_len, char* password, int password_len, char *encrypt, int encrypt_len);
int set_sta(char* ssid, int ssid_len, char* password, int password_len, char *encrypt, int encrypt_len);
void bakup_ap();
void restore_ap();
void reset_ap();
void bakup_sta();
void restore_sta();
void reset_sta();
#endif

