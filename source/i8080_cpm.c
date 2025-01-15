#include <stdlib.h>
#include "../include/i8080_debug.h"

const char separator[] = "\n\n--------------------------------------------------";

bool handle_cpm_calls(i8080_t* machine, FILE* ofp) {
    const uint16_t pc = i8080_register_get(machine, I8080_REGISTER_PROGRAM_COUNTER);
    
    // HLT (terminate)
    if (machine->mem[pc] == 0x76) {
        fputs(separator, ofp);
        fprintf(ofp, "HLT instruction encountered at %04Xh, terminating.\n", pc);
        return true;
    }

    // JMP 00 (terminate)
    if (pc == 0x00) {
        fputs(separator, ofp);
        fprintf(ofp, "User program returned control to CP/M, terminating.\n");
        return true;
    }

    // supervisor call
    if (pc == 0x05) {
        const uint16_t svc = i8080_register_get(machine, I8080_REGISTER_C);
        switch (svc) {
            case 2:
                // print character in register A
                fputc(i8080_register_get(machine, I8080_REGISTER_A), ofp);
                break;
            case 9:;
                // print string found at $DE and terminating with '$'
                uint16_t address = i8080_register_get(machine, I8080_REGISTER_DE);
                char c;
                do {
                    c = machine->mem[address];
                    fputc(c, ofp);
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