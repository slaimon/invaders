#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "safe.h"
#include "bytestream.h"
#include "i8080.h"
#include "i8080_disassembler.h"


void usage(const char* name) {
    printf("Records the instructions executed by an i8080 CPU and writes them\n");
    printf("to a file. The output file is called \"instr.bin\" by default.\n");
    printf("Usage: %s ROM_FILE [CPM|INV] NUM_ITER\n", name);
    printf("\tROM_FILE path to ROM file to execute.\n");
    printf("\tCPM|INV execution mode: either CPM for a CP/M program or INV for Space Invaders.\n");
    printf("\tNUM_ITER number of iterations to execute.\n");
}

size_t log_pos = 0;
bool log_append(bytestream_t* log, const i8080_t* cpu, i8080_instruction_t instr) {
    int instr_size = instr.instructionLength;
    if (log_pos + instr_size >= log->size) {
        return false; // log overflow
    }
    const uint8_t* ptr = &cpu->mem[instr.position];
    for (int i = 0; i < instr_size; i++)
        log->data[log_pos + i] = ptr[i];

    log_pos += instr_size;
    return true;
}

int main(int argc, char** argv) {
    if (argc != 4) {
        usage(argv[0]);
        return -1;
    }
    
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
    } else if (strcmp(argv[2], "INV") == 0) {
        i8080_memory_write(&cpu, *program, 0);
    } else {
        printf("Please select an execution mode between CPM and INV.\n");
        usage(argv[0]);
        return -1;
    }
    bytestream_destroy(program);

    size_t num_iter = strtoul(argv[3], NULL, 0);
    bytestream_t* log = bytestream_new(3 * (num_iter + 1) * sizeof(uint8_t));
    for (size_t i = 0; i < num_iter; i++) {
        i8080_instruction_t instr = disassemble_instruction(cpu.mem, cpu.programCounter);
        if (!log_append(log, &cpu, instr)) {
            printf("Halting early: log ran out of memory!\n");
            break;
        }

        int result = i8080_execute(&cpu);
        if (result < 0) {
            printf(
                "Halting early due to %s\n",
                result == I8080_HALT ?
                "HALT instruction" :
                "CPU failure"
            );
            break;
        }
    }
    
    FILE* ofp = safe_fopen("instr.bin", "wb");
    fwrite(log->data, sizeof(uint8_t), log_pos, ofp);
    fclose(ofp);
    return 0;
}