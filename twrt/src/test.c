#include <stdio.h>
//#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "buffer_ex.h"

void main(){
    //buffer_ring_byte test
    
    buffer_ring_byte *buff;
    buff = buffer_ring_byte_create(buff, 16);
    char a[9] = "123456789";
    char *b;
    char *c;
    //b = realloc(b, 7);
    //b[1] = 'a';
    printf("Read=%s\n",b);
    int m;
    for(m = 0; m < 16; m++){
	buffer_ring_byte_put(buff,a,9);
	b = buffer_ring_byte_get(buff,b,3);
	c = buffer_ring_byte_read(buff,b,9);
	printf("Len=%d\t",buffer_ring_byte_len(buff));
	printf("Buff=%s\t",buff->data);
	printf("Get=%s\t",b);
	printf("Read=%s\n",c);
    }

}

