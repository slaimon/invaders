#include <stdlib.h>

#include "../include/safe.h"
#include "../include/bytestream.h"
#include "../include/i8080_cpm.h"
#include "../include/i8080_debug.h"
#include "../include/i8080_disassembler.h"

#define FORMAT \
"N:%lu | %s\t | A:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X SP:%04X PC:%02X s:%1u z:%1u a:%1u p:%1u c:%1u | $HL:%02X $SP:%02X\n"
// example:
// N:6 | 21 C0 20       LXI   HL, #20C0 | A:FF B:1A C:00 D:00 E:01 H:5B L:0F SP:FFEC PC:0019 s:0 z:1 a:0 p:1 c:0 | $HL:9E $SP:4F
void printstate(const size_t iteration, const i8080_t machine, FILE* ofp) {
    char* currentInstructionDisassembly = i8080_instruction_toString(disassemble_instruction(machine.mem, machine.programCounter), false).string;
    fprintf(ofp, FORMAT, iteration, currentInstructionDisassembly,
        machine.A, machine.B, machine.C, machine.D, machine.E, machine.H, machine.L, machine.stackPointer, machine.programCounter,
        machine.signFlag, machine.zeroFlag, machine.auxCarryFlag, machine.parityFlag, machine.carryFlag, 
        machine.mem[i8080_register_get(&machine, I8080_REGISTER_HL)], machine.mem[machine.stackPointer]);
}

// Takes two filenames as arguments, one input and one output. The input file is loaded into memory
// and executed like a CPU program. The second file is used to dump a printout of the machine's
// state at each step of execution.
int main (int argc, char** argv) {
    if (argc != 4 && argc != 5) {
        printf("usage: %s ROM_FILE  OUTPUT_FILE  MAX_ITERATIONS  PAGE_NUM\n", argv[0]);
        return 0;
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
    
    size_t total_iteration = 0;
    size_t iteration = 0;
    size_t current_page = 0;
    size_t max_iter, page_num;
    sscanf(argv[3], "%lu", &max_iter);
    if (argc == 5) sscanf(argv[4], "%lu", &page_num);
    else page_num = 0;
    while (true) {
        if (current_page == page_num)
            printstate(total_iteration, machine, ofp);

        if (handle_cpm_calls_file(&machine, stdout))
            return 0;
        
        i8080_execute(&machine);
        if (current_page == page_num) {
            if (iteration > max_iter)
                return 0;
        }
        else
            if (iteration > max_iter) {
                ++current_page;
                iteration = 0;
                continue;
            }
        ++iteration;
        ++total_iteration;
    }

    return 0;
}