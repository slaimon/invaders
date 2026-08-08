#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "safe.h"
#include "bytestream.h"
#include "i8080.h"
#include "i8080_disassembler.h"


void usage(const char* name) {
    printf("Prints to stdout the instructions executed by an i8080 CPU.\n");
    printf("Redirect them to a file or to an audio device because why not.\n");
    printf("Usage: %s ROM_FILE [CPM|INV] NUM_ITER\n", name);
    printf("\tROM_FILE path to ROM file to execute.\n");
    printf("\tCPM|INV execution mode: either CPM for a CP/M program or INV for Space Invaders.\n");
    printf("\tNUM_ITER number of iterations to execute.\n");
}

void print_instr(i8080_instruction_t instr, const i8080_t* cpu) {
    fwrite(&cpu->mem[cpu->programCounter], sizeof(uint8_t), instr.instructionLength, stdout);
}

int main(int argc, char** argv) {
    if (argc != 4) {
        usage(argv[0]);
        return -1;
    }

    bool cpm_mode = false;
    
    i8080_t cpu;
    i8080_init(&cpu);

    // Load program (it's high time we wrote some abstractions for this)
    FILE* ifp = safe_fopen(argv[1], "rb");
    bytestream_t* program = bytestream_read(ifp);
    if (program == NULL) {
        printf("no valid program found.\n");
        return -1;
    }
    fclose(ifp);
    if (strcmp(argv[2], "CPM") == 0) {
        i8080_memory_write(&cpu, *program, 0x100);
        cpu.mem[5] = 0xC9;
        cpu.programCounter = 0x100;
        cpm_mode = true;
    } else if (strcmp(argv[2], "INV") == 0) {
        i8080_memory_write(&cpu, *program, 0);
    } else {
        printf("Please select an execution mode between CPM and INV.\n");
        usage(argv[0]);
        return -1;
    }
    bytestream_destroy(program);

    size_t num_iter = strtoul(argv[3], NULL, 0);
    for (size_t i = 0; i < num_iter; i++) {
        i8080_instruction_t instr = disassemble_instruction(cpu.mem, cpu.programCounter);
        print_instr(instr, &cpu);

        int result = i8080_execute(&cpu);
        if (result < 0) {
            printf(
                "Halting early due to %s",
                result == I8080_HALT ?
                    "HALT instruction" :
                    "CPU failure"
            );
            break;
        }
    }

    return 0;
}