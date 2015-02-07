#include "simple_serial.h"

struct_serial* serial_config(char* name, int type, char* baudrate, struct_serial* serial){
    strcpy(name, cfg->serial_name);
    serial->type = type;
    strcpy(serial->baudrate, cfg->serial_baudrate);
    return serial;
}

int serial_open(struct_serial* serial){
    int fd;//file descriptor
    struct termios tio;//terminal io setting

    fd = open(serial->name, O_RDWR | O_NOCTTY | O_NDELAY);//non-block serial port
    if(fd < 0){
	perror("serial open failed\n");
	return -1;
    }
    if(fcntl(fd, F_SETFL, FNDELAY) < 0){
	perror("fcntl set failed\n");
	return -1
    }// set serial into non-blocking mode
    if(isatty(STDIN_FILENO) == 0 ){
        printf("standard input is not a terminal device\n");
    }
   
    bzero(&tio, sizeof(tio)); //reset the tio

    //set serial mode
    switch(serial->type){
	case SERIAL_TYPE_UART:
	    tio.c_iflag = IGNPAR | ICRNL;
	    tio.c_oflag = 0;
	    tio.c_cflag |= CLOCAL | CREAD; // local operation, 8 bytes
	    tio.c_cflag &= ~CSIZE;
	    tio.c_cflag |= CS8;
	    tio.c_cc[VTIME] = 0;
	    tio.c_cc[VMIN] = 1;
	    break;
	case SERIAL_TYPE_SPI:
	    perror("unsupported serial type");
	    return -1;
	case SERIAL_TYPE_USB:
	    perror("unsupported serial type");
	    return -1;
	default:
	    perror("unsupported serial type");
	    return -1;
    }
    
    //set baudrate
    if(!strcmp(serial->baudrate, "9600")){
	    cfsetospeed(&tio, B9600);
	    cfsetispeed(&tio, B9600);
    }else if(!strcmp(serial->baudrate, "57600")){
	    cfsetospeed(&tio, B57600);
	    cfsetispeed(&tio, B57600);
    }else if(!strcmp(serial->baudrate, "115000")){
	    cfsetospeed(&tio, B115000);
	    cfsetispeed(&tio, B115000);
    }else{
	perror("unsupported serial baudrate");
	return -1;
    }

    tcflush(fd, TCIOFLUSH); //flush input & output

    if(tcgetattr(fd,serial->tio_bak)!=0){
	perror("serial set error\n");
	return -1;
    } //bak up previous tio

    if(tcsetattr(fd, TCSAFLUSH, &tio)!=0){
	perror("serial set error\n");
	return -1;
    } //set tio to fd after all data are flushed

    serial->fd = fd;//save fd
    serial->tio = tio; //save current tio
    return 0;//open sucessfully, return;
}

int serial_close(struct_serial* serial){
    //flush serial
    tcsetattr(serial->fd, TCSANOW, serial->tio_bak); //restore tio
    return close(serial->fd);
}
