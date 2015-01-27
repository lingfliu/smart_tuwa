#include "twrt.h"

int main(int argn, char* argv[]){

    get_config(&cfg);
    serial_config(&serial);
    inet_client_config(&inet_client);

    //initialize buffers
    buffer_serial = create_buffer_byte_ring(BUFF_RING_LEN);
    buffer_inet_client = create_buffer_byte_ring(BUFF_RING_LEN);
    //initialize queue
    creat_message_queue(msg_queue);


    pthread_create(thrd_serial_rx, run_serial_rx, NULL);
    pthread_create(thrd_inet_client_rx, run_inet_client, NULL);
    for(;;);
}

void *run_serial_rx(){
    int len;
    while(1){
	usleep(1);
	len = read(serial->fd, read_serial, SERIAL_BUFF_LEN);//non-blocking reading, return immediately
	if(len>0){
	    //lock here
	    put_buffer_byte_ring(buffer_serial, read_serial, len);
	    pthread_cond_signal(&cond_serial_input);
	    //unlock here
	}else if(len == 0){
	    //continue
	}else{
	    //if i/o errror
	    serial_close(serial);
	    while(serial_open(serial) < 0)
		usleep(5000); //wait 5 ms and try open serial again
	}
    }
}

void *run_inet_client_rx(){
    int len;
    while(1){
	usleep(1);
	len = read(int_client->fd, read_inet_client, INET_BUFF_LEN);//non-blocking reading, return immediately
	if(len>0){
	    //lock here
	    put_buffer_byte_ring(buffer_inet_client, read_inet_client, len);
	    pthread_cond_signal(&cond_inet_client_input);
	    //unlock here
	}else if(errno == EAGAIN | errno == EINTR ){
	    //continue
	}else if(len == 0){//if disconnected, reconnect
	    while(inet_client_connect(inet_client)<0)
		sleep(1);// wait 1 s and try connect again
	}else{
	    while(inet_client_connect(inet_client)<0)
		sleep(1);// wait 1 s and try connect again
	}
    }
}
