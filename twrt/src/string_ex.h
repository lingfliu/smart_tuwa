#ifndef _STRING_EX_
#define _STRING_EX_

#include <stdlib.h>
#include <string.h>
char* strtrim(char* str);//trim white spaces on head and tail of a string
char* strsub(char* str, char* substr, int start, int stop); //get substring from start to stop
int stridx(char* str, int idx_start, char parser); //get index of parse in a string
#endif
