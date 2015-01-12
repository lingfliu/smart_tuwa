#ifndef _SIMPLE_SERIAL_
#define _SIMPLE_SERIAL_

//Constants

typedef struct{
    char* name;
    int type;
    int baudrate;
    int parity;
}serial_config;

static char* buff_tx;
static char* buff_rx;
static int is_close;
int serial_init();
int on_serial_rx(int len, char* buff_rx);
int on_serial_tx(int len, char* buff_tx);
int serial_close();

#endif
