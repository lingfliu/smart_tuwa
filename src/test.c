#include "string_ex.h"
#include <stdio.h>
int main(int argn, char* argv[]){
    char str[20] = "hello world";
    char substr[10];
    printf("%s\n",str);
    printf("-%s-\n",strtrim(str));
    printf("-%s-\n",strtrim(strsub(str,substr,5,10)));
    printf("-%d-\n",stridx(str, 10, 'r'));
    return 0;
}
