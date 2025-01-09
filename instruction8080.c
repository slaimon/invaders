#include <stdlib.h>
#include "instruction8080.h"

i8080_line_t instruction8080_toString(instruction8080_t instruction) {
    i8080_line_t line;
    char* result = line.string;
    size_t pointer = 0;

    // address gutter on the left
    pointer += sprintf(&result[pointer], "%04X:\t", instruction.position);

    // binary representation of the instruction
    pointer += sprintf(&result[pointer], "%02X", instruction.opcode);
    for(int i = 0; i < instruction.num_inputValues; ++i)
        pointer += sprintf(&result[pointer], " %02X", abs(instruction.inputValues[i]));

    // the instruction's mnemonic
    if (instruction.num_inputValues < 2) {
        result[pointer] = '\t';
        ++pointer;
    }
    pointer += sprintf(&result[pointer], "\t%s", instruction.mnemonic);

    // register arguments, if any
    if (instruction.num_inputRegisters > 0) {
        if (instruction.num_inputValues == 0)
            pointer += sprintf(&result[pointer], "\t%s", instruction.inputRegisters);
        else
            pointer += sprintf(&result[pointer], "\t%s, ", instruction.inputRegisters);
    }
    else {
        result[pointer] = '\t';
        ++pointer;
    }
    
    // numerical arguments, if any
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
        pointer += sprintf(&result[pointer], "%c%02X%02X", prefix, first, second);
    }
    else if (instruction.num_inputValues == 1) {
        char prefix = instruction.immediate ? '#' : '$';
        pointer += sprintf(&result[pointer], "%c%02X", prefix, abs(instruction.inputValues[0]));
    }

    result[pointer++] = '\n';
    result[pointer] = '\0';

    if (pointer > I8080_LINE_LENGTH) {
        fprintf(stderr, "ERROR: buffer overrun while converting instruction to string\n");
        exit(-1);
    }

    return line;
}