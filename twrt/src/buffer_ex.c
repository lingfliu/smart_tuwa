#include "buffer_ex.h"


/////////////////////////////////////////////////
//Byte ring
////////////////////////////////////////////////
buffer_byte_ring* create_buffer_byte_ring(int len){
    buffer_byte_ring* buff = malloc(sizeof(buffer_byte_ring));
    buff->data = malloc(sizeof(char)*len);
    buff->p_head = buff->data;
    buff->p_tail = buff->data+len-1;
    buff->p_c = buff->data;
    buff->p_rw = buff->data;
    buff->overflow = 0;
    buff->len = len;
    return buff;
}

void destroy_buffer_byte_ring(buffer_byte_ring* buff){
    free(buff->data);
    free(buff);
    buff = NULL;
}

void put_buffer_byte_ring(buffer_byte_ring* buff, char* bytes, int len){
    int m;    
    for(m=0; m<len; m++){	
	*(buff->p_c) = bytes[m];
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
void put_buffer_byte_ring_one(buffer_byte_ring* buff, char* byte){
    *(buff->p_c) = byte[0];
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

int get_buffer_byte_ring(buffer_byte_ring* buff, char* bytes, int len){
    int m;
    int len_available;
    if(!buff->overflow)//if not overflow: p_rw<p_c
	len_available = buff->p_c-buff->p_rw;
    else
	len_available = buff->p_tail-buff->p_rw+buff->p_c-buff->p_head+1;//if overflow: p_rw > p_c:w
    len = len>len_available?len_available:len;

    if(len == 0)
	return -1;

    for(m=0; m<len; m++){
	bytes[m] = *(buff->p_rw);
	buff->p_rw++;
	if(buff->p_rw>buff->p_tail){
	    buff->p_rw = buff->p_head;
	    buff->overflow = 0;
	}
    }
    return 0;
}

int get_buffer_byte_ring_one(buffer_byte_ring* buff, char* byte){
    int len_available;
    if(!buff->overflow)//if not overflow: p_rw<p_c
	len_available = buff->p_c-buff->p_rw;
    else
	len_available = buff->p_tail-buff->p_rw+buff->p_c-buff->p_head+1;//if overflow: p_rw > p_c:w
    if(len_available==0){
	return -1;
    }else{
	byte[0] = *(buff->p_rw);
	buff->p_rw++;
	if(buff->p_rw>buff->p_tail){
	    buff->p_rw = buff->p_head;
	    buff->overflow = 0;
	}
	return 0;
    }
}

int len_buffer_byte_ring(buffer_byte_ring* buff){
    if(!buff->overflow)//if not overflow: p_rw<p_c
	len = buff->p_c-buff->p_rw;
    else
	len = buff->p_tail-buff->p_rw+buff->p_c-buff->p_head+1;//if overflow: p_rw > p_c:w
    return len;
}
/////////////////////////////////////////////////
//Number ring
////////////////////////////////////////////////
buffer_num_ring* create_buffer_num_ring(int len){
    buffer_num_ring* buff = malloc(sizeof(buffer_num_ring));
    buff->data = malloc(sizeof(float)*len);
    buff->p_head = buff->data;
    buff->p_tail = buff->data+len-1;
    buff->p_c = buff->data;
    buff->p_rw = buff->data;
    buff->overflow = 0;
    buff->len = len;
    return buff;
}

void destroy_buffer_num_ring(buffer_num_ring* buff){
    free(buff->data);
    free(buff);
    buff = NULL;
}

void put_buffer_num_ring(buffer_num_ring* buff, float* nums, int len){
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
void put_buffer_num_ring_one(buffer_num_ring* buff, float* num){
    *(buff->p_c) = num[0];
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

int get_buffer_num_ring(buffer_num_ring* buff, float* nums, int len){
    int m;
    int len_available;
    if(!buff->overflow)//if not overflow: p_rw<p_c
	len_available = buff->p_c-buff->p_rw;
    else
	len_available = buff->p_tail-buff->p_rw+buff->p_c-buff->p_head+1;//if overflow: p_rw > p_c:w
    len = len>len_available?len_available:len;

    if(len == 0)
	return -1;

    for(m=0; m<len; m++){
	nums[m] = *(buff->p_rw);
	buff->p_rw++;
	if(buff->p_rw>buff->p_tail){
	    buff->p_rw = buff->p_head;
	    buff->overflow = 0;
	}
    }
    return 0;
}
int get_buffer_num_ring_one(buffer_num_ring* buff, float* num){
    int len_available; 
    if(!buff->overflow)//if not overflow: p_rw<p_c len_available = buff->p_c-buff->p_rw;
	len_available = buff->p_c-buff->p_rw;
    else
	len_available = buff->p_tail-buff->p_rw+buff->p_c-buff->p_head+1;//if overflow: p_rw > p_c:w
    if(len_available==0){
	return -1;
    }else{
	num[0] = *(buff->p_rw);
	buff->p_rw++;
	if(buff->p_rw>buff->p_tail){
	    buff->p_rw = buff->p_head;
	    buff->overflow = 0;
	}
	return 0;
    }
}


int len_buffer_num_ring(buffer_num_ring* buff){
    if(!buff->overflow)//if not overflow: p_rw<p_c
	len = buff->p_c-buff->p_rw;
    else
	len = buff->p_tail-buff->p_rw+buff->p_c-buff->p_head+1;//if overflow: p_rw > p_c:w
    return len;
}
