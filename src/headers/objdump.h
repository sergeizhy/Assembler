#ifndef _OBJDUMP_H
#define _OBJDUMP_H
#include "utilities.h"

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

boolean objdump(IC* ,DC*,char*);

#endif
