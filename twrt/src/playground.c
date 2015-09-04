#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
	obj.val = 0;
	void *retval;

	char *name = "hello";
	int m;
	for (m = 0; m < 5; m ++) {
		obj.name[m] = name[m];
	}

	long a = 112323;
	char str[4];
	memcpy((void*) str, (void*) &a, 4*sizeof(char));
	printf("a = %ld\n", a);
	a--;
	memcpy((void*) &a, (void*) str, 4*sizeof(char));
	printf("a = %ld\n", a);

	while(0) {
		obj.val ++;
		if( pthread_create(&thr, NULL, run, (void*) &obj) < 0 )
			return;
		pthread_join(thr, &retval);
		printf("obj.val = %d", obj.val);
		printf(" thread returns %d\n", (* ((int*)retval)));
	}
}

void *run(void* arg){
	object *obj = (object *) arg;
	int *val;
	val = &(obj->val);
	pthread_exit((void*) val);
}
