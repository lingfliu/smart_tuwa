#include <stdio.h>
#include <stdlib.h>

typedef struct d_list d_list;
typedef struct d_list{
    int val;
    d_list* prev;
    d_list* next;
}d_list;

int main(int argn, char* argv[]){
	int a = 128;
	char c[4];
	memcpy(c,(char*) &a,4);
	printf("%d %d\n",(unsigned int)c[0],(unsigned int) c[1]);

	char *strs = "AADD0000AA00DD";
	int res = !memcmp(strs, "AADD",4);
	printf("%d\n",res);
	return 0;
}
