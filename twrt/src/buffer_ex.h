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
}buffer_byte_ring;

typedef struct{
    float* data;
    int len;
    float* p_head;
    float* p_tail;
    float* p_c; //current position of the buff
    float* p_rw; //read / write position
    int overflow; //flag indicating overflow of buff
}buffer_num_ring;


buffer_byte_ring* buffer_byte_ring_create(buffer_byte_ring* buff, int len);
void buffer_byte_ring_flush(buffer_byte_ring* buff);
void buffer_byte_ring_put(buffer_byte_ring* buff, char* bytes, int len);
void buffer_byte_ring_put_one(buffer_byte_ring* buff, char* byte);
int buffer_byte_ring_get(buffer_byte_ring* buff, char* bytes, int len);
int buffer_byte_ring_get_one(buffer_byte_ring* buff, char* byte);
void buffer_byte_ring_read(buffer_byte_ring* buff, char* bytes, int len);

int buffer_byte_ring_len(buffer_byte_ring* buff);//len available

buffer_num_ring* buffer_num_ring_create(buffer_num_ring* buff, int len);
void buffer_num_ring_flush(buffer_num_ring* buff);
void buffer_num_ring_put(buffer_num_ring* buff, float* nums, int len);
void buffer_num_ring_put_one(buffer_num_ring* buff, float* num);
int buffer_num_ring_get(buffer_num_ring* buff, float* nums, int len);
int buffer_num_ring_get_one(buffer_num_ring* buff, float* num);

int buffer_num_ring_len(buffer_num_ring* buff);//len available 
#endif
