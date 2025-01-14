#include <stdlib.h>

#include "../include/safe.h"
#include "../include/bytestream.h"
#include "../include/i8080.h"

char separator[] = "\n\n--------------------------------------------------";

// returns true iff the emulator should halt
bool handle_cpm_calls(i8080_t* machine) {
    uint16_t pc = i8080_register_get(machine, I8080_REGISTER_PROGRAM_COUNTER);
    
    // HLT (terminate)
    if (machine->mem[pc] == 0x76) {
        puts(separator);
        printf("HLT instruction encountered at %04Xh, terminating.\n", pc);
        return true;
    }

    // JMP 00 (terminate)
    if (pc == 0x00) {
        puts(separator);
        printf("User program returned control to CP/M, terminating.\n");
        return true;
    }

    // supervisor call
    if (pc == 0x05) {
        uint16_t svc = i8080_register_get(machine, I8080_REGISTER_C);
        switch (svc) {
            case 2:
                // print character in register A
                putchar(i8080_register_get(machine, I8080_REGISTER_A));
                break;
            case 9:;
                // print string found at $DE and terminating with '$'
                uint16_t address = i8080_register_get(machine, I8080_REGISTER_DE);
                char c;
                do {
                    c = machine->mem[address];
                    putchar(c);
                    ++address;
                } while (c != '$');
                break;

            default:
                fprintf(stderr, "\nERROR: unimplemented supervisor call: %04Xh\nFound at address: %04Xh\n", svc, pc);
                exit(EXIT_FAILURE);
        }
    }

    return false;
}

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
        return 0;
    }
    fclose(ifp);

    i8080_t machine;
    i8080_init(&machine);
    i8080_memory_write(&machine, *program, 0x100);  // CP/M loads user programs at address 100h
    bytestream_destroy(program);

    machine.mem[5] = 0xC9;  // Return immediately after all CP/M calls
    i8080_register_set(&machine, I8080_REGISTER_PROGRAM_COUNTER, 0X100);    // JMP 100h
    
    while (true) {
        if (handle_cpm_calls(&machine))
            return 0;
        
        i8080_execute(&machine);
    }

    return 0;
}