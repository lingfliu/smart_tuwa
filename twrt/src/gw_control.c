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
		return -1;
	}
	else {
		if(fread(key, sizeof(char), 8, fp)== 8){
			fclose(fp);
			return 8;
		}
		else {
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
