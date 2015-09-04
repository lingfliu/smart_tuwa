#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>


typedef struct{
	int val;
	char name[10];
} object;
void* run();

int main(int argn, char * arg[]){
	pthread_t thr;
	object obj;
	obj.val = 10;
	void *retval;

	char *name = "hello";
	int m;
	for (m = 0; m < 5; m ++) {
		obj.name[m] = name[m];
	}

	if( pthread_create(&thr, NULL, run, (void*) &obj) < 0 )
		return;
	pthread_join(thr, &retval);
	printf("the thread return result %c\n", * ((char*)retval));
}

void *run(void* arg){
	object *obj = (object *) arg;
	char *val = malloc(1);
	*val = 'a';
	pthread_exit((void*) val);
}
