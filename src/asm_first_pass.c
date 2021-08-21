#include "headers/asm_first_pass.h"

/**
 * Check delimiters between two data chunks 
 */
boolean parse_delimiters(const char *must, char **_line_buffer, size_t arguments, size_t line)
{
    char *_buffer = *_line_buffer;
    boolean comma = FALSE;
    while (*_buffer)
    {

        if (cispunct(*_buffer, must))
        {
            if (!isspace(*_buffer) && *_buffer == ',')
                comma = TRUE;
        }
        else if (isalpha(*_buffer) || isdigit(*_buffer) || cispunct(*_buffer, "\".$-+"))
        {
            if (!comma && arguments > 0)
                fprintf(err, "Syntax error on line %d: Missing comma\n", line + 1);
            else if (comma && arguments == 0)
                fprintf(err, "Syntax error on line %d: Illegal comma after instruction\n", line + 1);
            break;
        }
        else
        {
            return FALSE;
        }
        _buffer++;
    }

    *_line_buffer = _buffer;

    return TRUE;
}

/**
 *  Fill dc memory segment with the right memory address coming after ic memory segment
 */
boolean fill_dc_memory(DC *dc, uint32_t icf)
{
    for (dc->curr_pos = 0; dc->curr_pos < dc->size; dc->curr_pos++)
        dc->table[dc->curr_pos]->address += icf;
    return TRUE;
}
/**
 * Create undefined data chunks
 */
Undefined_data *create_undefined(Attributes attr, Type type, const char *label, Data *data, size_t line)
{
    Undefined_data *undefined = NULL;
    SAFE_ALLOC(undefined, calloc(1, sizeof *undefined));
    undefined->attr |= attr;
    undefined->data = data;
    undefined->type |= type;
    undefined->line = line;
    strcpy(undefined->buffer, label);
    return undefined;
}

/**
 *  Read chunks out of a buffer
 *  the function trims the _line_buffer and returns buffered String with a chunk of data to be later processed
 */
String parse_data(char **_line_buffer, boolean *reg, size_t line){

    String buffer = {0};
    boolean data = FALSE;
    char *_buffer = *_line_buffer;
    *reg = FALSE;

    SAFE_ALLOC(buffer.string, calloc(1, sizeof *buffer.string));
    buffer.size = 1;
    buffer.curr_pos = 0;

    while (isalpha(*_buffer) || cispunct(*_buffer, "\".$-+") || isdigit(*_buffer)){
        if (cispunct(*_buffer, "-+$")){
            data = TRUE;
            if (buffer.curr_pos != 0)
                fprintf(err, "Syntax error on line %d: incorrect data format\n", line + 1);
            else if (*_buffer == '$')
            {
                *reg = TRUE;
                _buffer++;
                continue;
            }
        }
        else if (data && !isdigit(*_buffer))
        {
            if (*reg)
                fprintf(err, "Syntax error on line %d: incorrect register format\n", line + 1);
            else
                fprintf(err, "Syntax error on line %d: incorrect data format\n", line + 1);
        }
        if (buffer.curr_pos + 1 > buffer.size)
            SAFE_ALLOC(buffer.string, realloc(buffer.string, ++buffer.size * sizeof *buffer.string));
            
        buffer.string[buffer.curr_pos++] = *_buffer++;
    }

    buffer.string[buffer.curr_pos] = '\0';
    *_line_buffer = _buffer;

    return buffer;
}

/**
 *  First pass 
 */
boolean asm_first_pass(File_descriptor *file, Buckets *symbols, Buckets instructions, DC *dc, IC *ic, Undefined *undefined){

    Symbols *symbol = NULL;
    Instructions *instruction = NULL;
    Data *data = NULL;
    String buffer = {NULL};
    boolean label = FALSE;
    boolean reg = FALSE;
    size_t line = 0;
    int val = 0;
    uint index = 26;
    uint arguments = 0;
    char *_buffer = NULL;
    char* format = NULL;

    HT_CREATE(symbols, file->size);
    SAFE_ALLOC(undefined->table, malloc(++undefined->size * sizeof *undefined->table));
    /**
     * Read lines out of the file struct
     */
    while (line < file->size)
    {

        _buffer = file->code_lines[line]->string;

        while (*_buffer){
            /**
             * label and data buffering
             * 
             * - Data from file will be buffered in a buffer to later be proccessed 
             * - incase there is a label the data will be categorized as a label, symbol struct will be created
             *   filled, in this stage symbol will be filled partially and pushed to the hashtable in a later phase
             *   with more information about the symbol
             * - Incase of any [errors] detected [errors] will pushed to [error].log file located in ../[error]/[error].log
             */

            buffer = parse_data(&_buffer, &reg, line);

            if (parse_delimiters(":", &_buffer, arguments, line))
            {

                if (!label){
                    if (FIND_ITEM(&instructions, (void **)&instruction, buffer.string) || FIND_ITEM(symbols, (void **)&symbol, buffer.string)){
                        if (instruction)
                            fprintf(err, "Syntax error on line %d: %s is a reserved key\n", line + 1, buffer.string);

                        else if (symbol){
                            if (!(symbol->attr & _extern) && !(symbol->attr & _entry))
                                fprintf(err, "Syntax error on line %d: %s already in use\n", line + 1, buffer.string);
                            else if (symbol->attr & _extern)
                                fprintf(err, "Syntax error on line %d: %s is an external label and cant be declared in the same file\n", line + 1, buffer.string);
                        }
                    }else{
                        SAFE_ALLOC(symbol, calloc(1, sizeof *symbol));
                        strcpy(symbol->name, buffer.string);
                    }
                }
                else
                    fprintf(err, "Syntax error on line %d: externous label\n", line + 1);

                label = TRUE;
            }
            else if (parse_delimiters(", \n", &_buffer, arguments, line))
            {

                /**
                 *    Instruction section
                 * 
                 *  - buffered instruction will be cattegorized and pushed to the instructions hashtable
                 *  - if a label was detected previously it will be also be cattegorized and pushed 
                 *    to symbols hashtable
                 *  - Data block will be created according to the attribute stored in the instructions hashtable
                 *    preloaded before the firstpass
                 *  - Data will be later pushed to the relevent data storage container according to the attribute to the given
                 *    attribute in the instruction
                 */

                if (arguments < 1){
                    if (FIND_ITEM(&instructions, (void **)&instruction, buffer.string)){

                        if ((instruction->attr & _data) || (instruction->attr & _code)){
                            if (instruction->attr & _code){
                                ALLOC_DATA(ic, data, 4, 100);
                                ic->curr_pos++;
                            }
                            else if (instruction->attr & _data){
                                ALLOC_DATA(dc, data, 1, 0);
                            }

                            WRITE_BITS(data->bits, instruction->opcode, 6, 26);
                            WRITE_BITS(data->bits, instruction->funct, 5, 6);
                        }
                        if (label && symbol){
                            symbol->address = &data->address;
                            symbol->attr |= instruction->attr;
                            if (!(symbol->attr & _entry))
                                ADD_ITEM(symbols, (void **)&symbol, symbol->name);
                        }
                        format = instruction->format;
                    }
                    else
                        fprintf(err, "Syntax error on line %d: instruction %s doesn't exist\n", line + 1, buffer.string);
                }
                else{
                    /**
                     *  Data
                     *  - Data will be processed and routed by previously detected instruction attribute
                     */
                    if (instruction){
                        if (instruction->attr & _extern || instruction->attr & _entry){

                            if (FIND_ITEM(&instructions, NULL, buffer.string))
                                fprintf(err, "Line %d: %s is a reserved key\n", line + 1, buffer.string);

                            else if (FIND_ITEM(symbols, (void **)&symbol, buffer.string)){
                                if (instruction->attr & _entry && !(symbol->attr & _extern))
                                    symbol->attr |= _entry;

                                else if (instruction->attr & _extern){
                                    fprintf(err, "Syntax error on line %d: %s label is already declared in the file cant be used as an external\n", line + 1, buffer.string);
                                }
                                else
                                    fprintf(err, "Syntax error on line %d: %s label pointed to an external label cant be assigned as an entry for this file\n", line + 1, buffer.string);
                            }
                            else{
                                SAFE_ALLOC(symbol, calloc(1, sizeof *symbol));
                                symbol->address = NULL;
                                symbol->attr |= instruction->attr;
                                strcpy(symbol->name, buffer.string);
                                ADD_ITEM(symbols, (void **)&symbol, symbol->name);
                            }
                        }
                        /**
                         * Data section
                         * - Route according to the instruction attribute
                         */
                        else if (instruction->attr & _data){

                            if (instruction->attr & _asciz){
                                if (buffer.string[0] == buffer.string[buffer.curr_pos - 1] && buffer.string[0] == '"'){
                                    for (buffer.curr_pos = 1; buffer.curr_pos < buffer.size; buffer.curr_pos++){
                                        ALLOC_DATA(dc, data, 1, 0);
                                        if (buffer.curr_pos != buffer.size - 1)
                                            WRITE_BITS(data->bits, buffer.string[buffer.curr_pos], 8, 0);
                                        data = NULL;
                                        dc->curr_pos++;
                                    }
                                }
                                else
                                    fprintf(err, "Syntax error on line %d: %s string missing quotation marks\n", line + 1, buffer.string);
                            }
                            else{
                                instruction->attr &= ~_data;

                                val = strtol(buffer.string, NULL, 10);
                                ALLOC_DATA(dc, data, 0, 0);

                                switch (instruction->attr)
                                {
                                case _db:
                                    if (val > INT8_MIN && val < INT8_MAX)
                                    {
                                        WRITE_BITS(data->bits, val, 8, 0);
                                        data->size = 1;
                                    }
                                    else
                                        fprintf(err, "Syntax error on line %d: %d is out of 8bit range\n", line + 1, val);
                                    break;
                                case _dh:
                                    if (val > INT16_MIN && val < INT16_MAX)
                                    {
                                        WRITE_BITS(data->bits, val, 16, 0);
                                        data->size = 2;
                                    }
                                    else
                                        fprintf(err, "Syntax error on line %d: $%d is out of 16bit range\n", line + 1, val);
                                    break;
                                case _dw:
                                    if (val > INT32_MIN && val < INT32_MAX)
                                    {
                                        WRITE_BITS(data->bits, val, 32, 0);
                                        data->size = 4;
                                    }
                                    else
                                        fprintf(err, "Syntax error on line %d: %d is out of 32bit range\n", line + 1, val);
                                    break;

                                default:
                                    fprintf(err, "Syntax error on line %d: Undefiend instruction\n", line + 1);
                                    break;
                                }
                                data = NULL;
                                instruction->attr |= _data;
                                dc->curr_pos++;
                            }
                        }
                        else{
                            /**
                             * Instruction data section
                             * - Route according to the instruction attribute
                             */
                            val = strtol(buffer.string, NULL, 10);

                            if (reg && (val < 0 || val > 32)){
                                fprintf(err, "Syntax error on line %d: $%s register value is out of range\n", line + 1, buffer.string);
                                break;
                            }
                            if(buffer.string && *format){
                                switch (instruction->type)
                                {
                                case I_instruction:
                                    if ((instruction->attr & _label) && arguments == 3){
                                        if (undefined->curr_pos + 1 > undefined->size)
                                            SAFE_ALLOC(undefined->table, realloc(undefined->table, ++undefined->size * sizeof *undefined->table));
                                        undefined->table[undefined->curr_pos++] = create_undefined(_label, I_instruction, buffer.string, data, line);
                                    }
                                    else{
                                        if (reg && *format == 'r'){
                                            WRITE_BITS(data->bits, val, 5, (index -= 5));
                                        }else
                                            WRITE_BITS(data->bits, val, 16, 0);
                                    }
                                    break;
                                case J_instruction:
                                    if (reg && *format == 'u'){
                                        WRITE_BITS(data->bits, val, 25, 0);
                                        WRITE_BITS(data->bits, reg, 1, 25);
                                    }
                                    else{
                                        if (undefined->curr_pos + 1 > undefined->size)
                                            SAFE_ALLOC(undefined->table, realloc(undefined->table, ++undefined->size * sizeof *undefined->table));
                                        undefined->table[undefined->curr_pos++] = create_undefined(_label, J_instruction, buffer.string, data, line);
                                    }
                                    break;
                                case R_instruction:
                                    if ((instruction->attr & _mv) && arguments == 2)
                                        index -= 5;
                                    WRITE_BITS(data->bits, val, 5, (index -= 5));
                                    break;
                                default:
                                    break;
                                }
                                format++;
                                
                            }else
                                fprintf(err,"Syntax error on line %d: externous operands/data\n",line + 1);
                        }
                    }
                }
                arguments++;
            }
            else
                fprintf(err, "Syntax error on line %d: invalid delimiters were found\n", line + 1);

        }
        if(format && *format)
            fprintf(err,"Syntax error on line %d: not enough operands\n",line + 1);
        
        instruction = NULL;
        symbol = NULL;
        format = NULL;
        data = NULL;
        label = FALSE;
        reg = FALSE;
        arguments = 0;
        index = 26;
        line++;
    }

    if (dc->size && ic->size)
        fill_dc_memory(dc, ic->table[ic->curr_pos - 1]->address + 4);

    return TRUE;
}