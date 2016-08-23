#include "gw_control.h"

void fan_control(int cmd){
	int fd;

	fd = open("/dev/fan_rt5350", O_RDWR);
	if (fd < 0) {
		printf("failed to open /dev/fan_rt5350\n");
	}
	else {
		if (cmd > 0){
			ioctl(fd,1);
		}
		else {
			ioctl(fd,0);
		}

		if (close(fd) < 0) {
			printf("failed to close /dev/fan_rt5350\n");
		}
	}
}

int get_password(char* key){
	FILE* fp;
	fp = fopen(FILE_PASSWORD, "r");
	if (fp == NULL){
		//printf("password file missing\n");
		return -1;
	}
	else {
		if(fread(key, sizeof(char), 8, fp)== 8){
			//printf("password found = %s\n", key);
			fclose(fp);
			return 8;
		}
		else {
			//printf("password not found \n");
			fclose(fp);
			return 0;
		}
	}
}

int set_password(char* key_prev, char* key_new){
	//authenticate key set request
	FILE* fp;
	char key[8];
	int res = get_password(key);
	int write_res = -1;
	if (res < 0) {
		//wrong configuration
		return res;
	}
	else if (res == 0){
		//key not set
		fp = fopen(FILE_PASSWORD, "w");
		if (fp == NULL){
			return -1;
		}
		else {
			write_res = fwrite(key_new, 8, sizeof(char), fp);
			fclose(fp);
			return write_res;
		}
	}
	else {
		//key set
		if (memcmp(key, key_prev, 8)){
			//prev key not matching, simply return
			return -1;
		}
		else {
			fp = fopen(FILE_PASSWORD, "w");
			if (fp == NULL){
				return -1;
			}
			else {
				write_res = fwrite(key_new, 8, sizeof(char), fp);
				fclose(fp);
				return write_res;
			}
		}
	}
}

int get_lic(char* lic){
	FILE *fp;
	fp = fopen(FILE_LIC, "r");
	if (fp == NULL){
		return -1;
	}
	else {
		if(fread(lic, sizeof(char), 16, fp)== 16){
			fclose(fp);
			return 16;
		}
		else {
			fclose(fp);
			return 0;
		}
	}
}

int set_lic(char* lic_prev, char* lic_new){
	//authenticate key set request
	FILE* fp;
	char lic[16];
	int res = get_lic(lic);
	int write_res = -1;
	if ( res <0 ) {
		//wrong configuration
		return res;
	}
	else if (res == 0){
		//key not set
		fp = fopen(FILE_LIC, "w");
		if (fp == NULL){
			return -1;
		}
		else {
			write_res = fwrite(lic_new, 16, sizeof(char), fp);
			fclose(fp);
			return write_res;
		}
	}
	else {
		//lic set
		if (memcmp(lic, lic_prev, 16)){
			return -1;
		}
		else {
			fp = fopen(FILE_LIC, "w");
			if (fp == NULL){
				return -1;
			}
			else {
				write_res = fwrite(lic_new, 16, sizeof(char), fp);
				fclose(fp);
				return write_res;
			}
		}
	}
}


int set_ap(char* ssid, int ssid_len, char* password, int password_len, char *encrypt, int encrypt_len){
	printf("set ap ssid=%s, key=%s\n", ssid, password);
	FILE *fp;
	int w_res;
	int i;

	char *ln[4] = {"config wifi-iface\n",
		"\toption device radio0\n",
		"\toption mode ap\n",
		"\toption network lan\n"};
	char *op_ssid = "\toption ssid ";
	char *op_encrypt= "\toption encryption psk2\n";
	char *op_key= "\toption key ";

	if (ssid_len == 0 || password_len == 0){
		reset_ap();
		system("ap_update&");
		return 0;
	}

	bakup_ap();

	fp = fopen(ETC_WIRELESS_COMPONENT_AP, "w+");
	if (fp == NULL){
		return -1;
	}
	else {
		printf("ap opened\n");
		//bakup wireless.ap
		for (i = 0; i < 4; i++){
			w_res = fwrite(ln[i], strlen(ln[i]), sizeof(char), fp);
			if (w_res < 0){
				//write fails
				fclose(fp);
				restore_ap();
				return -1;
			}
		}

		w_res = fwrite(op_ssid, strlen(op_ssid), sizeof(char), fp);
		if (w_res < 0){
				fclose(fp);
				restore_ap();
				return -1;
		}

		ssid[ssid_len] = '\n';
		w_res = fwrite(ssid, ssid_len+1, sizeof(char), fp);
		if (w_res < 0){
				fclose(fp);
				restore_ap();
				return -1;
		}

		w_res = fwrite(op_encrypt, strlen(op_encrypt), sizeof(char), fp);
		if (w_res < 0){
				fclose(fp);
				restore_ap();
				return -1;
		}

		w_res = fwrite(op_key, strlen(op_key), sizeof(char), fp);
		if (w_res < 0){
				fclose(fp);
				restore_ap();
				return -1;
		}


		password[password_len] = '\n';
		w_res = fwrite(password, password_len+1, sizeof(char), fp);
		if (w_res < 0){
				fclose(fp);
				restore_ap();
				return -1;
		}

		fclose(fp);

		system("ap_update");

		return 0;
	}
}

int set_sta(char* ssid, int ssid_len, char* password, int password_len, char *encrypt, int encrypt_len){
	FILE *fp;
	int w_res;
	int i;

	char *ln[4] = {"config wifi-iface\n",
		"\toption device radio0\n",
		"\toption mode sta\n",
	    "\toption network wwan\n"};
	char *op_ssid = "\toption ssid ";
	char *op_encrypt= "\toption encryption psk2\n";
	char *op_key= "\toption key ";

	if (ssid_len == 0 || password_len == 0){
		reset_sta();
		system("sta_update&");
		return 0;
	}

	bakup_sta();

	fp = fopen(ETC_WIRELESS_COMPONENT_STA, "w+");
	if (fp == NULL){
		return -1;
	}
	else {
		//bakup wireless.ap
		for (i = 0; i < 4; i++){
			w_res = fwrite(ln[i], strlen(ln[i]), sizeof(char), fp);
			if (w_res < 0){
				//write fails
				fclose(fp);
				restore_sta();
				return -1;
			}
		}

		w_res = fwrite(op_ssid, strlen(op_ssid), sizeof(char), fp);
		if (w_res < 0){
				fclose(fp);
				restore_sta();
				return -1;
		}

		ssid[ssid_len] = '\n';
		w_res = fwrite(ssid, ssid_len+1, sizeof(char), fp);
		if (w_res < 0){
				fclose(fp);
				restore_sta();
				return -1;
		}

		w_res = fwrite(op_encrypt, strlen(op_encrypt), sizeof(char), fp);
		if (w_res < 0){
				fclose(fp);
				restore_sta();
				return -1;
		}

		w_res = fwrite(op_key, strlen(op_key), sizeof(char), fp);
		if (w_res < 0){
				fclose(fp);
				restore_sta();
				return -1;
		}


		password[password_len] = '\n';
		w_res = fwrite(password, password_len+1, sizeof(char), fp);
		if (w_res < 0){
				fclose(fp);
				restore_sta();
				return -1;
		}

		fclose(fp);

		system("sta_update");

		return 0;
	}
}

void bakup_ap(){
	system("cp /root/wireless.ap /root/wireless.ap.bak");
}
void restore_ap(){
	system("cp /root/wireless.ap.bak /root/wireless.ap");
}
void reset_ap(){
	system("cp /root/wireless.ap.origin /root/wireless.sta");
}
void bakup_sta(){
	system("cp /root/wireless.sta /root/wireless.sta.bak");
}
void restore_sta(){
	system("cp /root/wireless.sta.bak /root/wireless.sta");
}
void reset_sta(){
	system("cp /root/wireless.sta.origin /root/wireless.sta");
}
