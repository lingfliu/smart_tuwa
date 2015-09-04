#include "utils.h"

long timediff_ms(struct timeval time_before, struct timeval time_after){
    long val;
	long sec = (time_after.tv_sec - time_before.tv_sec)*1000; 
	long usec = (time_after.tv_usec - time_before.tv_usec)/1000;
	val = sec+usec;
    return val;
}

long timediff_s(struct timeval time_before, struct timeval time_after){
	return time_after.tv_sec - time_before.tv_sec; 
}

long timediff_h(struct timeval time_before, struct timeval time_after){
    long val;
	val = (time_after.tv_sec - time_before.tv_sec)/3600; 
    return val;
}
