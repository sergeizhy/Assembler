#include "headers/utilities.h"

char* create_file(const char* format,...){
	String file = {NULL};
	char* buffer;
	va_list args;
	va_start(args,format);
	file.size = 1;
	file.curr_pos = 0;
	SAFE_ALLOC(file.string,malloc(file.size * sizeof *file.string));
	file.string[0] = '\0';
	while(*format){
		buffer = va_arg(args,char *);
		file.size += strlen(buffer);
		SAFE_ALLOC(file.string,realloc(file.string,file.size * sizeof *file.string));
		strcat(file.string,buffer);
		format++;
	}
	va_end(args);
	return file.string;
}

boolean cispunct(char ch,const char* delimiters){
	while (*delimiters){
		if(*delimiters == ch)
			return TRUE;
		delimiters++;
	}
	return FALSE;
}
size_t file_size(FILE* fp){
    size_t size = 0;
    if(fp){
        fseek(fp,0L,SEEK_END);
        size = ftell(fp);
        fseek(fp,0L,SEEK_CUR);
    }
    return size;
}