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

#define SERIAL_TYPE_UART 0
#define SERIAL_TYPE_SPI 1
#define SERIAL_TYPE_USB 2

#define SERIAL_BUFF_LEN 255 //buffer length for write/read

typedef struct{
    char name[100];
    int fd;
    int type;
    int baudrate;
    struct termios tio;
    struct termios tio_bak;
}struct_serial;

int serial_open(struct_serial* serial);
int serial_close(struct_serial* serial);
int on_serial_rx(struct_serial *serial, char* rx_buff);
int on_serial_tx(struct_serial *serial, char* tx_buff, int len);

#endif
