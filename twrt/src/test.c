#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_EDIT "/home/liulingfeng/install"

typedef struct {
	char id[8];
	int type;
	char descrip[50];
	int len_descrip;
}install_t;

void readfile();
void writefile();

int main(int argn, char ** argv) {
	writefile();
	readfile();
}

void writefile(){
	FILE* fp;
	char str[256];
	char strTmp[256];
	int idx[3];
	int idxCnt;
	int cnt;
	int m;
	int len_descrip;

	install_t list[10];

	fp = fopen(FILE_EDIT, "w+");
	if (fp == NULL) {
		return;
	}

	char id[8] = "12345678";
	int type = 32;
	char *descrip = "kitchen";
	
	for (m = 0; m  < 10; m ++) {
		fwrite(id, 8, sizeof(char), fp);
		fprintf(fp, ":");
		fprintf(fp, "%d", type);
		fprintf(fp, ":");
		fprintf(fp, "%s", descrip);
		fprintf(fp, ":");
		if (m < 9) {
			fprintf(fp, "\n");
		}
		type ++;
	}

	fclose(fp);
}

void readfile(){
	FILE* fp;
	char str[256];
	char strTmp[256];
	int idx[3];
	int idxCnt;
	int cnt;
	int m;
	int len_descrip;

	install_t list[10];
	
	fp = fopen(FILE_EDIT, "r");
	if (fp == NULL) {
		//file not exist, create and close
		fp = fopen(FILE_EDIT, "w+");
		fclose(fp);
		return;
	}
	else {
		cnt = 0;
		while( fgets(str, 512, fp) != NULL) {
			idxCnt = 0;
			for (m = 0; m < 512; m ++){
				if ( str[m] == ':') {
					idx[idxCnt] = m;
					idxCnt++;
					if (idxCnt > 3){
						break;
					}
				}
			}

			memcpy(list[cnt].id, str, sizeof(char)*idx[0]);
			printf("id is %s\n", list[cnt].id);

			bzero(strTmp, sizeof(char)*256);
			memcpy(strTmp, str+idx[0]+1, sizeof(char)*(idx[1]-idx[0]-1));
			sscanf(strTmp, "%d", &(list[cnt].type));
			printf("type is %s\n", strTmp);

			bzero(strTmp, sizeof(char)*256);
			memcpy(strTmp, str+idx[1]+1, sizeof(char)*(idx[2]-idx[1]-1));
			printf("description is %s\n", strTmp);
			len_descrip = strlen(strTmp);
			memcpy(strTmp, list[cnt].descrip, sizeof(char)*len_descrip);

			cnt ++;
		}
		fclose(fp);
	}
}
