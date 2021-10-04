#include "headers/asm_file_handler.h"
#include "headers/asm_first_pass.h"
#include "headers/asm_second_pass.h"
#include "headers/objdump.h"

#define CONTAINER_FREE(container)                      			\
	{                                                  		\
		container.curr_pos = 0;                        		\
		while (container.curr_pos < container.size)    		\
		{                                              		\
			FREE(container.table[container.curr_pos]); 	\
			container.curr_pos++;                      	\
		}                                              		\
		FREE(container.table);                         		\
		container.curr_pos = 0;                        		\
		container.size = 0;                            		\
	}

void FREE_FILE(File_descriptor *fd)
{
	size_t index = 0;
	for (; index < fd->size; index++)
	{
		FREE(fd->code_lines[index]->string);
		FREE(fd->code_lines[index]);
	}
	FREE(fd->code_lines);
	fd->size = 0;
}

/**
 * create hashtable with all the necessary attributes and data
 * 
 */
boolean instructions_ht(Buckets *instructions){
	size_t index;

	Instructions *_t_instructions = NULL;
	Instructions table[33] = {
		{"add", 1, 0,"rrr", _code, R_instruction},{"sub", 2, 0,"rrr", _code, R_instruction}, 
		{"and", 3, 0,"rrr", _code, R_instruction},{"or", 4, 0,"rrr", _code, R_instruction}, 
		{"nor", 5, 0,"rrr", _code, R_instruction},{"move", 1, 1,"rr", _code | _mv, R_instruction}, 
		{"mvhi", 2, 1,"rr", _code | _mv, R_instruction},{"mvlo", 3, 1,"rr", _code | _mv, R_instruction}, 
		{"addi", 0, 10,"rir", _code, I_instruction},{"subi", 0, 11,"rir", _code, I_instruction}, 
		{"andi", 0, 12,"rir", _code, I_instruction},{"ori", 0, 13,"rir", _code, I_instruction}, 
		{"nori", 0, 14,"rir", _code, I_instruction},{"bne", 0, 15,"rri", _code | _brnch | _label, I_instruction}, 
		{"beq", 0, 16,"rri", _code | _brnch | _label, I_instruction},{"blt", 0, 17,"rri", _code | _brnch | _label, I_instruction}, 
		{"bgt", 0, 18,"rri", _code | _brnch | _label, I_instruction},{"lb", 0, 19,"rir", _code | _mem, I_instruction}, 
		{"sb", 0, 20,"rir", _code | _mem, I_instruction},{"lw", 0, 21,"rir", _code | _mem, I_instruction}, 
		{"sw", 0, 22,"rir", _code | _mem, I_instruction},{"lh", 0, 23,"rir", _code | _mem, I_instruction}, 
		{"sh", 0, 24,"rir", _code | _mem, I_instruction},{"jmp", 0, 30,"u", _code | _label, J_instruction}, 
		{"la", 0, 31,"u", _code | _label, J_instruction},{"call", 0, 32,"u", _code | _label, J_instruction}, 
		{"stop", 0, 63,"", _code, none},{".entry", 0, 0,"", _entry, none}, 
		{".extern", 0, 0,"", _extern, none},{".dw", 0, 0,"", _data | _dw, none}, 
		{".dh", 0, 0,"", _data | _dh, none},{".db", 0, 0,"", _data | _db, none},
		{".asciz", 0, 0,"", _data | _asciz, none}};
	/*hashtable */
	HT_CREATE(instructions, 15);

	for (index = 0; index < sizeof(table) / sizeof(table[0]); index++)
	{
		_t_instructions = malloc(sizeof *_t_instructions);
		memcpy(_t_instructions, &table[index], sizeof(*_t_instructions));
		ADD_ITEM(instructions, (void **)&_t_instructions, _t_instructions->name);
	}
	return TRUE;
}
/**
 * create a 'File descriptor' and read file 
 * - clean any unnecessary delimiters
 * - prepare the file for the first pass
 */
boolean parse_file(File_descriptor *file, FILE *fp)
{
	boolean data_stream = FALSE;
	String *buffer = NULL;
	size_t size = 1;
	char ch = 0;

	SAFE_ALLOC(buffer, calloc(1, sizeof *buffer));
	SAFE_ALLOC(file->code_lines, calloc(size, sizeof *file->code_lines));
	SAFE_ALLOC(buffer->string, calloc(BUFFER_SIZE, sizeof *buffer->string));

	file->size = 0;
	buffer->size = BUFFER_SIZE;

	while (!feof(fp)){

		ch = fgetc(fp);

		if (ch == '\n' || ch == EOF){
			if (buffer->curr_pos > 0){
				buffer->string[buffer->curr_pos] = '\n';
				file->code_lines[file->size++] = buffer;
				if (ch != EOF){
					if (file->size + 1 > size){
						SAFE_ALLOC(file->code_lines, realloc(file->code_lines, ++size * sizeof *file->code_lines));
					}
					SAFE_ALLOC(buffer, calloc(1, sizeof *buffer));
					SAFE_ALLOC(buffer->string, calloc(BUFFER_SIZE, sizeof *buffer->string));
				}
			}
			data_stream = FALSE;
		}
		else{
			if (!data_stream && ch != ' '){
				/*escape comment lines*/
				if (ch == ';'){
					continue;
				}
				else if (!isalpha(ch) && ch != '.'){
					fprintf(err, "Syntax error on line %d: wrong line beginning\n", file->size + 1);
					break;
				}
				data_stream = TRUE;
			}
			if ((buffer->curr_pos + 1) < BUFFER_SIZE && data_stream){
				if (ch == '\t')
					ch = ' ';
				if (cispunct(ch, "\",$:") && buffer->string[buffer->curr_pos - 1] == ch){
					fprintf(err, "Syntax error on line %d: externous delimiters\n", file->size + 1);
					break;
				}
				else if (ch == ':' && !isalpha(buffer->string[buffer->curr_pos - 1])){
					fprintf(err, "Syntax error on line %d: Label has an incorrect form\n", file->size + 1);
					break;
				}
				else if (ch == ' ' && (buffer->string[buffer->curr_pos - 1] == ' ' || cispunct(buffer->string[buffer->curr_pos - 1], "\",$:")))
					continue;
				else if (!isalpha(ch) && !cispunct(ch, "\",$: -.+") && !isdigit(ch)){
					fprintf(err, "Syntax error on line %d: unallowed character %c\n", file->size + 1, ch);
					break;
				}
				else if (ch == ',' && buffer->string[buffer->curr_pos - 1] == ' ')
					buffer->string[buffer->curr_pos - 1] = ch;
				else
					buffer->string[buffer->curr_pos++] = ch;
			}
			else if (data_stream){
				fprintf(err, "Syntax error on line %d: excceeds 80 characters\n", file->size + 1);
				return FALSE;
			}
		}
	}
	return TRUE;
}

boolean assembler(char **argv, int argc){

	Buckets symbols = {NULL};
	Buckets instructions = {NULL};
	Undefined undefined = {NULL};
	File_descriptor file = {NULL};
	DC dc = {NULL};
	IC ic = {NULL};
	FILE *asm_file = NULL;
	char *extention = NULL;

	instructions_ht(&instructions);

	for (; argc >= 1; --argc){

		extention = (argv[argc] + strlen(argv[argc])) - 3;
		if (strcmp(extention, ".as") != 0){
			fprintf(stderr, "File error extention does not much required file extention '.as'\n");
			return FALSE;
		}

		OPEN_FILE(create_file("*", argv[argc]), "r", asm_file);
		fprintf(stdout, "\t\tAssembling file %s\n", argv[argc]);

		*extention = '\0';
		OPEN_FILE(create_file("**", argv[argc], ".err"), "w+", err);

		parse_file(&file, asm_file);
		asm_first_pass(&file, &symbols, instructions, &dc, &ic, &undefined);
		if (asm_second_pass(&undefined, &symbols, argv[argc]) && objdump(&ic, &dc, argv[argc]))
			printf("\t\tFile assembled successfully\n\n");
		else{
		    fprintf(stderr, "Errors were detected in the first pass aborting, please refer to %s%s\n\n", argv[argc], ".err");
			rewind(err);
			printf("%s",err->_IO_read_ptr);
		}

		HT_FREE(&symbols);
		FREE_FILE(&file);
		CLOSE_FILE(asm_file);
		CONTAINER_FREE(ic);
		CONTAINER_FREE(dc);
		CONTAINER_FREE(undefined);
		CLOSE_FILE(ent);
		CLOSE_FILE(ext);
		CLOSE_FILE(ob);
		CLOSE_FILE(err);
		extention = NULL;
	}
	HT_FREE(&instructions);

	return TRUE;
}
