#include <stdio.h>
//#include <time.h>
//#include <unistd.h>
#include <stdlib.h>

void main(){
    /*
    time_t now;
    struct tm *tmutc, *tmlocal;
    time(&now);
    tmutc = gmtime(&now);
    printf("%s \n", asctime(tmutc));
    long x; 
    while(1){
	usleep(1000);
	printf("%ld \n", x+=10000);
    }*/
    char a[10];
    char *b = malloc(sizeof(char)*10);
    printf("%d\n", sizeof(b));
}
