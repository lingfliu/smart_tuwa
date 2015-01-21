#ifndef _SIMPLE_SERIAL_
#define _SIMPLE_SERIAL_

//Constants

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

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
void on_serial_rx(char* rx_buff, int fp);
void on_serial_tx(char* rx_buff, int fp);
void serial_cfg_bak(struct_serial* serial);
void serial_cfg_restore(struct_serial* serial);

#endif
