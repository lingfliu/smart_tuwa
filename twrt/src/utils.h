#ifndef _UTILS_
#define _UTILS_

#include <sys/time.h>

//time functions
/*get the time difference in milli-seconds*/
long timediff_ms(struct timeval time1, struct timeval time2);
/*get the time difference in seconds*/
long timediff_s(struct timeval time1, struct timeval time2);
/*get the time difference in hours*/
long timediff_h(struct timeval time1, struct timeval time2);

#endif
