#ifndef _BUFFER_EX_
#define _BUFFER_EX_

#include <stdlib.h>

typedef struct{
    char* data;
    int len;
    char* p_head;
    char* p_tail;
    char* p_c; //current position of the buff
    char* p_rw; //read/write position
    int overflow; //flag indicating overflow of buff
}buffer_ring_byte;

typedef struct{
    float* data;
    int len;
    float* p_head;
    float* p_tail;
    float* p_c; //current position of the buff
    float* p_rw; //read / write position
    int overflow; //flag indicating overflow of buff
}buffer_ring_num;


buffer_ring_byte* buffer_ring_byte_create(buffer_ring_byte* buff, int len);
void buffer_ring_byte_flush(buffer_ring_byte* buff);
buffer_ring_byte* buffer_ring_byte_del(buffer_ring_byte* buff);
void buffer_ring_byte_put(buffer_ring_byte* buff, char* bytes, int len);
char* buffer_ring_byte_get(buffer_ring_byte* buff, char* bytes, int len);
char* buffer_ring_byte_read(buffer_ring_byte* buff, char* bytes, int len);

int buffer_ring_byte_len(buffer_ring_byte* buff);//len available

buffer_ring_num* buffer_num_byte_create(buffer_ring_num* buff, int len);
void buffer_ring_num_flush(buffer_ring_num* buff);
buffer_ring_num* buffer_ring_num_del(buffer_ring_num* buff);
void buffer_ring_num_put(buffer_ring_num* buff, float* nums, int len);
float* buffer_ring_num_get(buffer_ring_num* buff, float* nums, int len);
float* buffer_ring_num_read(buffer_ring_num* buff, float* nums, int len);

int buffer_ring_num_len(buffer_ring_num* buff);//len available 
#endif
