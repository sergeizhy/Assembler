#include "headers/asm_second_pass.h"

/**
 *  Second pass 
 */
boolean asm_second_pass(Undefined *undefined_data, Buckets *symbols, char *base_file)
{
    Symbols *symbol = NULL;
    Undefined_data *undefine = NULL;
    uint32_t data = 0;
    size_t s_data = 0;
    size_t index = 0;
    
    if (file_size(err_file))
        return FALSE;
    else{
        /** 
         *  Check for undefined labels
         *  - Fill ext file with found extentions
         *  - Fill ent file with found enteries
         */
        while (index < undefined_data->size){
            undefine = undefined_data->table[index];

            if (FIND_ITEM(symbols, (void **)&symbol, undefine->buffer)){
                if (undefine->attr & _label){

                    if (symbol && symbol->address){
                        switch (undefine->type){
                        case J_instruction:
                            s_data = 24;
                            data = *symbol->address;
                            break;
                        case I_instruction:
                            s_data = 16;
                            data = *symbol->address - undefine->data->address;
                            break;
                        default:
                            break;
                        }
                        WRITE_BITS(undefine->data->bits, data, s_data, 0);
                    }
                }
                if (symbol->attr & _extern){
                    if (!ext)
                        OPEN_FILE(create_file("**", base_file, ".ext"), "wr", ext);
                    fprintf(ext, "%s %04d\n", undefine->buffer, undefine->data->address);
                }
            }
            else
                fprintf(err, "Syntax error on line %d: There is no such symbol %s\n", undefine->line + 1, undefine->buffer);
            index++;
        }
        /*Iterate entries from symbol table*/
        for (index = 0; index < symbols->capacity; index++)
        {
            while (symbols->item[index])
            {
                if (((Symbols *)symbols->item[index]->data)->attr & _entry)
                {
                    if (!ent)
                        OPEN_FILE(create_file("**", base_file, ".ent"), "wr", ent);
                    fprintf(ent, "%s %04d\n", ((Symbols *)symbols->item[index]->data)->name, *((Symbols *)symbols->item[index]->data)->address);
                }
                symbols->item[index] = symbols->item[index]->next;
            }
        }
    }

    return TRUE;
}