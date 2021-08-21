#include "headers/objdump.h"

#define PRINT_DATA(container,address){                                                          \
    size_t index;                                                                               \
    container->curr_pos = 0;                                                                    \
    while(container->curr_pos < container->size){                                               \
        fprintf(ob,"%04d ",address += 4);                                                       \
        for(index = 0; index < 4  && container->curr_pos < container->size; index++){           \
            fprintf(ob,"%02X ",(char)container->table[container->curr_pos]->bits & 0xFF);       \
            container->table[container->curr_pos]->bits >>= 8;                                  \
            if(!--container->table[container->curr_pos]->size)                                  \
                container->curr_pos++;                                                          \
        }                                                                                       \
        fprintf(ob,"\n");                                                                       \
    }                                                                                           \
}

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