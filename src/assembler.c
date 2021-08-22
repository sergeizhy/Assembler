#include "headers/asm_file_handler.h"

#include "headers/utilities.h"
int main(int argc,char** argv){
    if(argc < 2){
        fprintf(stderr,"Too little arguments in command line\n");    
        exit(EXIT_FAILURE);
    }
    assembler(argv,argc - 1);

    return 0;
}  