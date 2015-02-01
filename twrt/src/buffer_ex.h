#ifndef _BUFFER_EX_
#define _BUFFER_EX_

#include <stdlib.h>

typedef struct{
    char* data;
    int len;
    char* p_head;
    char* p_tail;
    char* p_c; //current position of the buff
    char* p_rw; //read / write position
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


buffer_byte_ring* create_buffer_byte_ring(int len);
void destroy_buffer_byte_ring(buffer_byte_ring* buff);
void put_buffer_byte_ring(buffer_byte_ring* buff, char* bytes, int len);
void put_buffer_byte_ring_one(buffer_byte_ring* buff, char* byte);
int get_buffer_byte_ring(buffer_byte_ring* buff, char* bytes, int len);
int get_buffer_byte_ring_one(buffer_byte_ring* buff, char* byte);
void read_buffer_byte_ring(buffer_byte_ring* buff, char* bytes, int len);

buffer_num_ring* create_buffer_num_ring(int len);
void destroy_buffer_num_ring(buffer_num_ring* buff);
void put_buffer_num_ring(buffer_num_ring* buff, float* nums, int len);
void put_buffer_num_ring_one(buffer_num_ring* buff, float* num);
int get_buffer_num_ring(buffer_num_ring* buff, float* nums, int len);
int get_buffer_num_ring_one(buffer_num_ring* buff, float* num);

int len_buffer_num_ring(buffer_num_ring* buff);//len available 
int len_buffer_byte_ring(buffer_byte_ring* buff);//len available
#endif
