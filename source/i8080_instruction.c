#include <stdlib.h>
#include "../include/i8080_instruction.h"

#define INSERT_WHITESPACE(n)        \
    for(int i = 0; i < n; ++i)      \
        result[pointer++] = ' ';

// the mnemonics are placed at this column
#define MNEMONIC_COLUMN_NUMBER  22
// the comments are placed at this column
#define COMMENT_COLUMN_NUMBER   40


i8080_line_t i8080_instruction_toString(i8080_instruction_t instruction) {
    i8080_line_t line;
    char* result = line.string;
    size_t pointer = 0;

    bool hasComment = (instruction.comment[0] != '\0');

    // address gutter on the left
    pointer += sprintf(&result[pointer], "%04X:  ", instruction.position);

    // binary representation of the instruction (or padding if needed)
    pointer += sprintf(&result[pointer], "%02X", instruction.opcode);
    for(int i = 0; i < instruction.num_inputValues; ++i) {
        pointer += sprintf(&result[pointer], " %02X", abs(instruction.inputValues[i]));
    }

    // the instruction's mnemonic
    int padding = MNEMONIC_COLUMN_NUMBER - pointer;
    INSERT_WHITESPACE(padding)
    pointer += sprintf(&result[pointer], instruction.mnemonic);

    // register arguments, if any
    INSERT_WHITESPACE(2)
    if (instruction.num_inputRegisters > 0) {
        if (instruction.num_inputValues == 0)
            pointer += sprintf(&result[pointer], instruction.inputRegisters);
        else
            pointer += sprintf(&result[pointer], "%s, ", instruction.inputRegisters);
    }
    else if (instruction.num_inputValues == 0 && hasComment)
        INSERT_WHITESPACE(4)
    
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
        INSERT_WHITESPACE(2)
    }

    // comment, if any
    if (hasComment) {
        padding = COMMENT_COLUMN_NUMBER - pointer;
        INSERT_WHITESPACE(padding)
        pointer += sprintf(&result[pointer], "; %s", instruction.comment);
    }

    result[pointer++] = '\n';
    result[pointer] = '\0';

    if (pointer > I8080_LINE_LENGTH) {
        fprintf(stderr, "ERROR: buffer overrun while converting instruction to string\n");
        exit(-1);
    }

    return line;
}