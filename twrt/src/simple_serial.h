#ifndef _SIMPLE_SERIAL_
#define _SIMPLE_SERIAL_

//Constants

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERIAL_TYPE_UART 1
#define SERIAL_TYPE_SPI 2
#define SERIAL_TYPE_USB 3

#define SERIAL_BUFF_LEN 255 //buffer length for write/read

typedef struct{
    char name[50];
    int fd;
    int type;
    char baudrate[20];
    struct termios tio;
    struct termios tio_bak;
}serial;

void serial_config(char* name, int type, char* baudrate, serial* srl);
int serial_open(serial* srl);
int serial_close(serial* serial);

#endif
