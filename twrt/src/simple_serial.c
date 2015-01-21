#include "simple_serial.h"

int serial_open(struct_serial* serial){
    int fd;//file descriptor
    struct termios tio;//terminal io setting
    fd = open(serial->name, O_RDWR | O_NOCTTY | O_NDELAY);
    if(fd < 0){
	perror("Cannot open serial port, exit.\n");
	return -1;
    }

    bzero(&tio, sizeof(tio)); //reset the tio

    //set serial mode
    switch(serial->type){
	case SERIAL_TYPE_UART:
	    tio.c_iflag = IGNPAR | ICRNL;
	    tio.c_oflag = 0;
	    tio.c_cflag = CS8 | CLOCAL | CREAD;
	    //tio.c_lflag = ;
	    tio.c_cc[VTIME] = 0;
	    tio.c_cc[VMIN] = 1;
	    //break;
	case SERIAL_TYPE_SPI:
	    //break;
	case SERIAL_TYPE_USB:
	    //break;
	default:
	    break;
    }
    
    //set baudrate
    switch(serial->baudrate){
	case 1:
	    cfsetospeed(&tio, B9600);
	    cfsetispeed(&tio, B9600);
	    break;
	case 2:
	    cfsetospeed(&tio, B57600);
	    cfsetispeed(&tio, B57600);
	    break;
	case 3:
	    cfsetospeed(&tio, B115000);
	    cfsetispeed(&tio, B115000);
	    break;
	default:
	    cfsetospeed(&tio, B9600);
	    cfsetispeed(&tio, B9600);
	    break;
    }

    tcgetattr(fd,serial->tio_bak); //bak up previous tio
    tcsetattr(fd, TCSAFLUSH, &tio); //set tio to fd after all data are flushed
    tcflush(fd, TCIOFLUSH); //flush input & output
    serial->fd = fd;
    serial->tio = tio; //save current tio
    return 0;
}

void on_serial_rx(char* rx_buff, int fd){
    int result;
    result = read(fd, rx_buff, SERIAL_BUFF_LEN);
}

int serial_close(struct_serial* serial){
    tcsetattr(serial->fd, TCSANOW, serial->tio_bak); //restore tio
    return close(serial->fd);
}
