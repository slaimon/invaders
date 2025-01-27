#include <stdlib.h>

#include "../include/safe.h"
#include "../include/bytestream.h"
#include "../include/i8080_cpm.h"

// Takes one filename as argument and executes it like a CP/M program. The ROM is loaded at the
// correct address in memory and some supervisor calls are handled, which allows the program to
// output text to the console.
int main (int argc, char** argv) {
    if (argc != 2) {
        printf("usage: %s ROM_FILE\n", argv[0]);
    }

    FILE* ifp = safe_fopen(argv[1], "rb");
    bytestream_t* program = bytestream_read(ifp);
        if (program == NULL) {
        printf("No valid program found\n");
        return -1;
    }
    fclose(ifp);

    i8080_t machine;
    i8080_init(&machine);
    i8080_memory_write(&machine, *program, 0x100);  // CP/M loads user programs at address 100h
    bytestream_destroy(program);

    machine.mem[5] = 0xC9;  // Return immediately after all CP/M calls
    machine.programCounter = 0x100;    // JMP 100h
    
    printf("Loaded ROM: %s\n", argv[1]);
    puts("--------------------------------------------------\n");
    size_t cycles = 0;
    while (true) {
        if (handle_cpm_calls(&machine))
            break;
        
        cycles += i8080_execute(&machine);
    }

    printf("Elapsed %lu cycles (%lf seconds @ 2MHz)\n\n", cycles, (double)cycles/2000000.0);
    return 0;
}