#include <stdint.h>
#include <stdlib.h>
#include "i8080.h"

const char separator[] = "\n\n--------------------------------------------------\n";

// print character in register A
static void print_char(const i8080_t* machine, FILE* ofp) {
    fputc(machine->A, ofp);
}

// print string found at $DE and terminating with '$'
static void print_string(const i8080_t* machine, FILE* ofp) {
    uint16_t address = i8080_get_de(machine);
    char c;
    do {
        c = machine->mem[address];
        if (c != '$')
            fputc(c, ofp);
        ++address;
    } while (c != '$');
}

static void fail_with_error(uint16_t svc, uint16_t address) {
    fprintf(stderr, "\nERROR: unimplemented supervisor call: %04Xh\nFound at address: %04Xh\n", svc, address);
    exit(EXIT_FAILURE);
}

// Acts in stead of the CP/M operating system using ofp as an output terminal
static void handle_supervisor_call(const i8080_t* machine, FILE* ofp) {
    uint16_t svc = machine->C;
    uint16_t pc = machine->programCounter;
    switch (svc) {
        case 2:
            print_char(machine, ofp);
            break;

        case 9:
            print_string(machine, ofp);
            break;

        default:
            fail_with_error(svc, pc);
    }
}

bool handle_cpm_calls_file(const i8080_t* machine, FILE* ofp) {
    const uint16_t pc = machine->programCounter;
    
    // terminate if we found a HLT instruction
    if (machine->mem[pc] == 0x76) {
        fputs(separator, ofp);
        fprintf(ofp, "HLT instruction encountered at %04Xh, terminating.\n", pc);
        return true;
    }

    switch (pc) {
        case 0x00:
            // terminate if we found JMP 00
            fputs(separator, ofp);
            fprintf(ofp, "User program returned control to CP/M, terminating.\n");
            return true;

        case 0x05:
        handle_supervisor_call(machine, ofp);
            return false;
        
        default:
            return false;
    }
}