#ifndef _ASM_FIRST_PASS_H
#define _ASM_FIRST_PASS_H
#include "utilities.h"
#include "hashtable.h"


#define PROG_SIZE(prg_size,dt_size){                                                                                            \
    if(prg_size + dt_size > MEM_PROG_SIZE){                                                                                     \
        prg_size += dt_size;                                                                                                    \
        if(err){                                                                                                                \
            fprintf(err,"Program exceeds limited size %s\n",STRINGIFY(MEM_PROG_SIZE));                                          \
            exit(EXIT_FAILURE);                                                                                                 \
        }                                                                                                                       \
    }                                                                                                                           \
}

#define ALLOC_DATA(container, dt, sz, addr){                                                                                    \
    if (dt == NULL){                                                                                                            \
        SAFE_ALLOC(dt, calloc(1, sizeof *dt));                                                                                  \
        dt->size = sz;                                                                                                          \
        if (container->curr_pos < 1)                                                                                            \
            dt->address = addr;                                                                                                 \
        else                                                                                                                    \
            dt->address = container->table[container->curr_pos - 1]->address + container->table[container->curr_pos - 1]->size; \
    }                                                                                                                           \
    if (container->curr_pos + 1 > container->size){                                                                             \
        SAFE_ALLOC(container->table, realloc(container->table, ++container->size * sizeof *container->table));                  \
        container->table[container->curr_pos] = dt;                                                                             \
    }                                                                                                                           \
}

boolean asm_first_pass(File_descriptor *, Buckets*, Buckets, DC *, IC *,Undefined *);

#endif