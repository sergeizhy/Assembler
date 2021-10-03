#include "headers/objdump.h"

boolean objdump(IC* ic,DC* dc,char* base_file){
    uint32_t icf;
    uint32_t dcf;

    if(file_size(err))
        return FALSE;
    else{
        icf = ic->table[ic->curr_pos - 1]->address + ic->table[ic->curr_pos - 1]->size;
        dcf = dc->table[dc->curr_pos - 1]->address + dc->table[dc->curr_pos - 1]->size;

        OPEN_FILE(create_file("**",base_file,".ob"),"wr",ob);
        fprintf(ob,"%7d %d\n",icf - 100, dcf - icf);
        icf = ic->table[0]->address - 4;
        PRINT_DATA(ic,icf);
        PRINT_DATA(dc,icf);
    }
    return TRUE;
}
