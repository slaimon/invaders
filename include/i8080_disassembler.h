#ifndef __I8080_DISASSEMBLER_H__
#define __I8080_DISASSEMBLER_H__

#include <stdbool.h>
#include "bytestream.h"

#define I8080_LITTLE_ENDIAN  true

#define I8080_MNEMONIC_NAME_LENGTH  4
#define I8080_REGISTER_NAME_LENGTH  8
#define I8080_MAX_COMMENT_LENGTH   64

#define I8080_LINE_LENGTH          64 + I8080_MAX_COMMENT_LENGTH

/*
    i8080_instruction_t describes an intel8080 instruction as found in a program, along with some
    information to facilitate human reading.

    An intel8080 instruction is composed of either one, two or three bytes of machine code.
    The first byte is always the opcode, possibly followed by either:
       - one byte representing an 8-bit numeric literal, or
       - two bytes representing a 16-bit numeric literal, or
       - two bytes representing a 16-bit memory address

    The first two kinds of instruction are called immediate, the other indirect.
*/
typedef struct {
    uint8_t opcode;
    char mnemonic[I8080_MNEMONIC_NAME_LENGTH + 1];
    uint16_t position;                   // address where the instruction is found
    unsigned short instructionLength;    // length of instruction in bytes (== num_inputValues + 1)
    
    // registers affected by instruction
    char inputRegisters[I8080_REGISTER_NAME_LENGTH + 1];
    unsigned short num_inputRegisters;    // number of registers affected by instruction
    
    // numerical arguments of the instruction (at most 2 bytes)
    uint8_t inputValues[2];
    unsigned short num_inputValues;      // number of numerical arguments of the instruction
    bool immediate;                      // true if the value is a literal, false if it's an address

    char comment[I8080_MAX_COMMENT_LENGTH];
} i8080_instruction_t;

// a fixed-width string containing a single line of i8080 assembly listing
typedef struct {
    char string[I8080_LINE_LENGTH + 1];
} i8080_line_t;

// prints one instruction to a fixed-width string
i8080_line_t i8080_instruction_toString(i8080_instruction_t instruction);



// returns a struct describing the instruction found at the specified address in the memory 
i8080_instruction_t disassemble_instruction(const uint8_t* mem , unsigned int addr);

// returns a string containing the text disassembly of the given program
void disassemble_program(bytestream_t program, FILE* ofp);

#endif