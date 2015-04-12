#include <stdio.h>
#include <stdlib.h>

typedef struct d_list d_list;
typedef struct d_list{
    int val;
    d_list* prev;
    d_list* next;
}d_list;

int main(int argn, char* argv[]){
    d_list queue;
    queue.val = 2;
    queue.prev = &queue;
    queue.next = &queue;
    printf("%d\n", queue.prev->val);
}
