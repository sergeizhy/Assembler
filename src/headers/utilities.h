#ifndef _UTILS_H
#define _UTILS_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
FILE* err_file;
FILE* ob_file;
FILE* ext_file;
FILE* ent_file;

#define err err_file
#define ent ent_file
#define ext ext_file
#define ob ob_file

#define BUFFER_SIZE 81

#define DEBUG_U (0)

#define MEM_PROG_SIZE 1<<25

#define STRINGIFY(msg)( #msg )

typedef unsigned int uint;
typedef enum boolean{FALSE,TRUE}boolean;


#define OPEN_FILE(file,mode,fp){										\
	fp = fopen((file),mode);											\
	if(fp == NULL){														\
		fprintf(stderr, "File error: %s failed to open\n", file);		\
		perror(NULL);													\
		exit(EXIT_FAILURE);												\
	}																	\
	free(file);															\
}	

#define CLOSE_FILE(file){												\
	if(file != NULL){													\
		fclose(file);													\
		file = NULL;													\
	}																	\
}																		

#define SAFE_ALLOC(ptr,alloc)                                       	\
do{                                                                 	\
    void *_n_ptr = NULL;												\
    	_n_ptr = alloc;                                                	\
    	if(_n_ptr == NULL){                                             \
        	fprintf(err, "FATAL: Failed to allocate memory\n");      	\
        	exit(EXIT_FAILURE);                                         \
    	}else                                                           \
        	ptr = _n_ptr;                                               \
}while(0);                                                          	

#define FREE(ptr){														\
	if(ptr != NULL){													\
		free(ptr);														\
		ptr = NULL;														\
	}																	\
}
#define PRINT_BITS(number){												\
	uint size = sizeof(uint);											\
    int i;																\
    for (i = size * 8 - 1; i >= 0; i--)									\
        printf("%u", (number >> i) & 1);								\
    printf("\n");														\
}

#define WRITE_BITS(dest,src,src_size,dest_pos){							\
    uint32_t mask = 0xFFFFFFFF;                                     	\
    (src) &= (mask >> ((sizeof(uint32_t) * 8) - src_size));         	\
    (dest) |= (src) << (dest_pos);             		                	\
	if DEBUG_U															\
		PRINT_BITS(dest);												\
}			                                                          	

#define PRNT_FILE(file){\
	\
}
typedef enum Type{
	none,
	R_instruction,
	J_instruction,
	I_instruction
}Type;

typedef enum Attributes{
	_asciz = 	1,
	_db = 		1<<1,
	_dh = 		1<<2,
	_dw = 		1<<3,
	_label =	1<<4,
	_extern =	1<<5,
	_entry =	1<<6,
	_code = 	1<<7,
	_data = 	1<<8,
	_mv =		1<<9,
	_mem = 		1<<10,
	_brnch =	1<<11
}Attributes;

typedef struct String { 
	char* string;
	size_t size;
	size_t curr_pos;
}String;

typedef struct Symbols{
	char name[BUFFER_SIZE];
	uint32_t *address;
	Attributes attr;
}Symbols;

typedef struct Instructions{
	char name[BUFFER_SIZE];
	int funct;
	int opcode;
	char* format;
	Attributes attr;
	Type type;
}Instructions;

typedef struct File_descriptor{
	String **code_lines;
	size_t size;
}File_descriptor;

typedef struct Data{
	uint32_t bits;
	uint32_t address;
	size_t size;
}Data;

typedef struct Instruction_counter{
	Data** table;
	size_t size;
	size_t curr_pos;
}IC;

typedef struct Data_counter{
	Data** table;
	size_t size;
	size_t curr_pos;
}DC;

typedef struct Undefined_data{
	char buffer[BUFFER_SIZE];
	Data* data;
	Attributes attr;
	size_t line;
	Type type;
}Undefined_data;

typedef struct Undefined{
	Undefined_data **table;
	size_t curr_pos;
	size_t size;
}Undefined;

boolean cispunct(char,const char*);
char* create_file(const char* ,...);
size_t file_size(FILE* );
#endif