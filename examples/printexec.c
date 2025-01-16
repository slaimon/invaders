#include <stdlib.h>

#include "../include/safe.h"
#include "../include/bytestream.h"
#include "../include/i8080_cpm.h"
#include "../include/i8080_debug.h"

// Takes two filenames as arguments, one input and one output. The input file is loaded into memory
// and executed like a CPU program. The second file is used to dump a printout of the machine's
// state at each step of execution.
int main (int argc, char** argv) {
    if (argc != 3) {
        printf("usage: %s ROM_FILE  OUTPUT_FILE\n", argv[0]);
    }

    FILE* ifp = safe_fopen(argv[1], "rb");
    FILE* ofp = safe_fopen(argv[2], "w");
    bytestream_t* program = bytestream_read(ifp);
        if (program == NULL) {
        printf("No valid program found\n");
        return 0;
    }
    fclose(ifp);

    i8080_t machine;
    i8080_init(&machine);
    i8080_memory_write(&machine, *program, 0x100);  // CP/M loads user programs at address 100h
    bytestream_destroy(program);

    machine.mem[5] = 0xC9;  // Return immediately after all CP/M calls
    i8080_register_set(&machine, I8080_REGISTER_PROGRAM_COUNTER, 0X100);    // JMP 100h
    
    size_t iteration = 0;
    while (true) {
        fprintf(ofp, "ITERATION No: %lu\n-------------------------------------\n\n", iteration++);
        i8080_printState(machine, 0, ofp);
        fputs("\n\n", ofp);

        if (handle_cpm_calls_file(&machine, stdout))
            return 0;
        
        i8080_execute(&machine);
    }

    return 0;
}