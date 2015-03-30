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

void buffer_ring_byte_create(buffer_ring_byte *buff, int len); //create ring buffer given size len
void buffer_ring_byte_flush(buffer_ring_byte* buff); //flush the bytes and reset the buffer
void buffer_ring_byte_del(buffer_ring_byte* buff); //del the ring buffer / remove data

void buffer_ring_byte_put(buffer_ring_byte* buff, char* bytes, int len); 
char* buffer_ring_byte_get(buffer_ring_byte* buff, char* bytes, int len);
char* buffer_ring_byte_read(buffer_ring_byte* buff, char* bytes, int len);

int buffer_ring_byte_getlen(buffer_ring_byte* buff);//len available
#endif
