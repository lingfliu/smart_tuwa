#include "string_ex.h"


//this function trim white spaces of a string's head and tail
char* strtrim(char* str){
    int n;
    int str_len = strlen(str);
    int idx_head = 0;
    int idx_tail = str_len-1;
    char* str_tmp = malloc(sizeof(char)*str_len);

    //null string
    if(str_len == 0)
	return NULL; 

    //trim from the head
    for(n = 0; n<str_len; n++){
	if(str[n]==' ' || str[n] == '\t' || str[n] == '\n'){
	    idx_head++;
	}
	else{
	    break;
	}
    }
    
    //if string only contains whitespace 
    if(idx_head == str_len)
	return NULL; 

    //trim from the tail
    for(n = str_len-1; n>=0; n--){
	if(str[n]==' ' || str[n] == '\t' || str[n] == '\n'){
	    idx_tail--;
	}
	else{
	    break;
	}
    }

    if(idx_head == 0 && idx_tail == str_len-1)
	return str;

    str[idx_tail+1] = '\0';
    
    for(n = idx_head; n<=idx_tail+1; n++)
	str[n-idx_head] = str[n];
    return str;
}

//function get substring from str with start and stop index
char* strsub(char* str, char* substr, int start, int stop){
    int n;
    int str_len = strlen(str);

    //check if paras are valid, if not, return empty string
    if(start>str_len-1 || stop<0 || stop<start){
	free(substr);
	substr = NULL;
	return substr;
    }

    for(n = start; n <=stop; n++)
	substr[n-start] = str[n];
    substr[n] = '\0';
    return substr;
}

//function to get the first appearance of c afte the idx start
int stridx(char* str, int idx_start, char c){
    int n;
    int str_len = strlen(str);
    int idx = -1;

    if(idx_start>=str_len || idx_start<0)
	return idx;

    for(n = idx_start; n<str_len; n++)
	if(str[n] == c){
	    idx = n;
	    break;
	}
    return idx;
}
