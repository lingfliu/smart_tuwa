#ifndef _STRING_EX_
#define _STRING_EX_

#include <stdlib.h>
#include <string.h>
char* strtrim(char* str);
char* strsub(char* str, char* substr, int start, int stop);
int stridx(char* str, int idx_start, char parser);
#endif
