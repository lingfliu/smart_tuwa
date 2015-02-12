#include "buffer_ex.h"


/////////////////////////////////////////////////
//Byte ring
////////////////////////////////////////////////
buffer_ring_byte* buffer_ring_byte_create(buffer_ring_byte* buff, int len){
    buff = malloc(sizeof(buffer_ring_byte));
    buff->data = realloc(buff->data, sizeof(char)*len);
    buff->p_head = buff->data;
    buff->p_tail = buff->data+len-1;
    buff->p_c = buff->data;
    buff->p_rw = buff->data;
    buff->overflow = 0;
    buff->len = len;
    return buff;
}

buffer_ring_byte* buffer_ring_byte_del(buffer_ring_byte* buff){
    free(buff->data);
    free(buff);
    buff = NULL;
    return buff;
}

void buffer_ring_byte_flush(buffer_ring_byte* buff){
    char a;
    while(buffer_ring_byte_len(buff)>0)
	buffer_ring_byte_get(buff,&a,1); //empty bytes in the buff
}

void buffer_ring_byte_put(buffer_ring_byte* buff, char* bytes, int len){
    int m;    
    for(m=0; m<len; m++){	
	*(buff->p_c) = bytes[m];
	buff->p_c++;//to the next empty position
	if(buff->p_c > buff->p_tail && buff->overflow == 0){
	    buff->p_c = buff->p_head;
	    buff->overflow = 1;
	}else if(buff->p_c > buff->p_tail && buff->overflow == 1){
	    buff->p_c = buff->p_head;
	    if(buff->p_rw == buff->p_tail)//move p_rw to p_c
		buff->p_rw = buff->p_c;

	}
	//push the p_rw so p_c < = p_rw and is overflowed
	if((buff->p_c > buff->p_rw) && buff->overflow == 1 )
	    buff->p_rw = buff->p_c;
    }
}

char* buffer_ring_byte_get(buffer_ring_byte* buff, char* bytes, int len){
    int m;
    int len_available=buffer_ring_byte_len(buff);
    if(len>len_available || len == 0)
	return bytes;

    bytes = realloc(bytes, sizeof(char)*len);

    for(m=0; m<len; m++){
	bytes[m] = *(buff->p_rw);
	buff->p_rw++;

	if(buff->p_rw > buff->p_tail){
	    buff->p_rw = buff->p_head;
	    buff->overflow = 0;
	}
    }
    return bytes;
}

char* buffer_ring_byte_read(buffer_ring_byte* buff, char* bytes, int len){
    if(len>buffer_ring_byte_len(buff) || len == 0)
	return bytes;
    bytes = realloc(bytes, len);
    int m;
    for(m = 0; m<len; m++){
	if(buff->p_rw+m<=buff->p_tail)
	    bytes[m] = *(buff->p_rw+m);
	else
	    bytes[m] = *(buff->p_head+m-(buff->p_tail-buff->p_rw)-1);
    }
    return bytes;
}

int buffer_ring_byte_len(buffer_ring_byte* buff){
    int len;
    if(!buff->overflow)//if not overflow: p_rw<p_c
	len = buff->p_c-buff->p_rw;
    else
	len = buff->p_tail-buff->p_rw+buff->p_c-buff->p_head+1;//if overflow: p_rw > p_c:w
    return len;
}

/////////////////////////////////////////////////
//Number ring
////////////////////////////////////////////////
buffer_ring_num* buffer_num_byte_create(buffer_ring_num* buff, int len){
    buff = malloc(sizeof(buffer_ring_num));
    buff->data = realloc(buff->data, sizeof(float)*len);
    buff->p_head = buff->data;
    buff->p_tail = buff->data+len-1;
    buff->p_c = buff->data;
    buff->p_rw = buff->data;
    buff->overflow = 0;
    buff->len = len;
    return buff;
}

buffer_ring_num* buffer_ring_num_del(buffer_ring_num* buff){
    free(buff->data);
    free(buff);
    buff = NULL;
    return buff;
}

void buffer_ring_num_flush(buffer_ring_num* buff){
    float a;
    while(buffer_ring_num_len(buff)>0)
	buffer_ring_num_get(buff,&a,1); //empty bytes in the buff
}

void buffer_num_byte_put(buffer_ring_num* buff, float* nums, int len){
    int m;    
    for(m=0; m<len; m++){	
	*(buff->p_c) = nums[m];
	buff->p_c++;//to the next empty position
	if(buff->p_c > buff->p_tail){
	    buff->p_c = buff->p_head;
	    buff->overflow = 1;
	    if(buff->p_rw == buff->p_tail)
		buff->p_rw = buff->p_c;
	}
	//push the p_rw so p_c < = p_rw and is overflowed
	if(buff->p_c > buff->p_rw && buff->overflow ==1)
	    buff->p_rw = buff->p_c;
    }
}

float* buffer_ring_num_get(buffer_ring_num* buff, float* nums, int len){
    int m;
    if(len>buffer_ring_num_len(buff) || len ==0)
	return nums;

    nums = realloc(nums, len);
    for(m=0; m<len; m++){
	nums[m] = *(buff->p_rw);
	buff->p_rw++;
	if(buff->p_rw>buff->p_tail){
	    buff->p_rw = buff->p_head;
	    buff->overflow = 0;
	}
    }
    return nums;
}

float* buffer_ring_num_read(buffer_ring_num* buff, float* nums, int len){
    if(len>buffer_ring_num_len(buff) || len == 0)
	return nums;
    nums = realloc(nums, len);
    int m;
    for(m = 0; m<len; m++){
	if(buff->p_rw+m<=buff->p_tail)
	    nums[m] = *(buff->p_rw+m);
	else
	    nums[m] = *(buff->p_head+m-buff->p_tail+buff->p_rw-2);
    }
    return nums;
}

int buffer_ring_num_len(buffer_ring_num* buff){
    int len;
    if(!buff->overflow)//if not overflow: p_rw<p_c
	len = buff->p_c-buff->p_rw;
    else
	len = buff->p_tail-buff->p_rw+buff->p_c-buff->p_head+1;//if overflow: p_rw > p_c:w
    return len;
}
