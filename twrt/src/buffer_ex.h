#ifndef _BUFFER_EX_
#define _BUFFER_EX_

#include <stdlib.h>
#include <string.h>

typedef struct{
    char* data;
    int len;
    char* p_head;
    char* p_tail;
    char* p_c; //current position of the buff
    char* p_rw; //read/write position
    int overflow; //flag indicating overflow of buff
}buffer_ring_byte;

void buffer_ring_byte_create(buffer_ring_byte *buff, int len); //create ring buffer given size len
void buffer_ring_byte_flush(buffer_ring_byte* buff); //flush the bytes and reset the buffer
void buffer_ring_byte_del(buffer_ring_byte* buff); //set ring buffer as length 0  

void buffer_ring_byte_put(buffer_ring_byte* buff, char* bytes, int len); //put data into the buffer
void buffer_ring_byte_get(buffer_ring_byte* buff, char* bytes, int len); //get and remove data from buffer
void buffer_ring_byte_read(buffer_ring_byte* buff, char* bytes, int len); //read without removing data from buffer


int buffer_ring_byte_getlen(buffer_ring_byte* buff);//get the actual data length in the buffer
#endif
