#include <stdio.h>
//#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef struct data_list{
    int a;
    struct data_list* next;
    struct data_list* prev;
}data_list;

void init_data_list(data_list* dl);
void put_data_list(data_list** dl, int a);
int get_data_list(data_list* dl);
int getlen_data_list(data_list* dl);

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
    //printf("%ld\n", sizeof(b));

    data_list* dl = malloc(sizeof(data_list));
    init_data_list(dl);
    int m;
    for(m=0; m< 20; m++)
	put_data_list(&dl, m);
    printf("%d\n",getlen_data_list(dl));
    //for(m=0; m< 10; m++)
	//printf("%d\n",get_data_list(dl));
    while(!isempty_data_list(dl))
	printf("%d\n",get_data_list(dl));

    char* c;
    c = realloc(c,sizeof(char)*5);
    c[0] = '1';
    c[1] = '2';
    c[2] = '3';
    c[3] = '4';
    c[4] = '5';

    printf("%s\n",c);
    memset(c, 0, 2);
    c = realloc(c,sizeof(char)*2);
    printf("%s\n",c);

    char aa = 0x08;
    int aaa = (int) (aa<<20);
    printf("size of int = %ld\n", sizeof(int));
    printf("size of unsigned int = %ld\n", sizeof(unsigned int));
    printf("size of char = %ld\n", sizeof(char));
    printf("size of float = %ld\n", sizeof(float));
    printf("%d\n", aaa);
}

void init_data_list(data_list* dl){
    dl->a = -1;
    dl->next = dl;
    dl->prev = dl;
}
void put_data_list(data_list** dl, int a){
    data_list* dl_item;
    if((*dl)->a ==-1)//null list
	(*dl)->a = a;
    else{
	dl_item = malloc(sizeof(data_list));
	dl_item->a = a;
	dl_item->prev = (*dl);
	dl_item->next = dl_item;
	(*dl)->next = dl_item;
	(*dl) = dl_item;
    }
}

int get_data_list(data_list* dl){
    data_list* dl_item = dl;
    int result;
    if(dl_item->prev == dl_item){//if only one in the list
	result = dl_item->a;
	dl_item->a = -1;
	return result;
    }
    while(dl_item->prev != dl_item)
	dl_item = dl_item->prev;
    result = dl_item->a;
    dl_item->next->prev = dl_item->next;
    free(dl_item);
    return result;
}

int getlen_data_list(data_list* dl){
    data_list* dl_item;
    dl_item = dl;
    int len = 0;
    while(dl_item->next != dl_item)
	dl_item = dl_item->next; 
    if(dl_item->prev == dl_item && ~isempty_data_list(dl))//if only one
	return 1;
    else if(dl_item->prev == dl_item && isempty_data_list(dl))//
	return len;
    else{
	while(dl_item->prev != dl_item){
	    len++;
	    dl_item = dl_item->prev;
	}
	len ++;//do forget the head
	return len;
    }
}

int isempty_data_list(data_list *dl){
    if(dl->prev ==dl && dl->next == dl && dl->a<0)
	return 1;
    else
	return 0;
}

