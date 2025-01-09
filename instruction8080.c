#include <stdlib.h>
#include "instruction8080.h"

#define MATH_ABS(x) \
            (((x) >= 0) ? (x) : (-(x)))

i8080_line_t instruction8080_toString(instruction8080_t instruction) {
    i8080_line_t line;
    char* result = line.string;
    size_t pointer = 0;

    // print the address gutter on the left
    pointer += sprintf(&result[pointer], "%04X:\t", instruction.position);

    // print the binary representation of the instruction
    pointer += sprintf(&result[pointer], "%02X", instruction.opcode);
    for(int i = 0; i < instruction.num_inputValues; ++i)
        pointer += sprintf(&result[pointer], " %02X", MATH_ABS(instruction.inputValues[i]));

    // print the instruction's mnemonic
    if (instruction.num_inputValues < 2) {
        result[pointer] = '\t';
        ++pointer;
    }
    pointer += sprintf(&result[pointer], "\t%s\t", instruction.mnemonic);

    // print the instruction's register arguments, if any
    if (instruction.num_inputRegisters > 0) {
        if (instruction.num_inputValues == 0)
            pointer += sprintf(&result[pointer], "%s", instruction.inputRegisters);
        else
            pointer += sprintf(&result[pointer], "%s, ", instruction.inputRegisters);
    }
    
    // print the instruction's numerical arguments
    // immediate arguments are prefixed with '#'
    // while addresses are prefixed with '$'
    if (instruction.num_inputValues == 2) {
        char prefix = instruction.immediate ? '#' : '$';
        #if I8080_LITTLE_ENDIAN
            uint8_t first = abs(instruction.inputValues[1]);
            uint8_t second = abs(instruction.inputValues[0]);
        #else
            uint8_t first = abs(instruction.inputValues[0]);
            uint8_t second = abs(instruction.inputValues[1]);
        #endif
        pointer += sprintf(&result[pointer], "%c%02X%02X\t", prefix, first, second);
    }

    result[pointer++] = '\n';
    result[pointer] = '\0';

    if (pointer > LINE_SIZE) {
        fprintf(stderr, "ERROR: buffer overrun while converting instruction to string\n");
        exit(-1);
    }

    return line;
}