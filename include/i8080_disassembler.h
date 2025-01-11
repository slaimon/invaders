#ifndef __I8080_DISASSEMBLER_H__
#define __I8080_DISASSEMBLER_H__

#include "bytestream.h"
#include "i8080_instruction.h"

#define I8080_DISASSEMBLER_LITTLE_ENDIAN  true


// returns a struct describing the instruction found at the specified address in the memory 
i8080_instruction_t disassemble_instruction(uint8_t* mem , unsigned int addr);

// returns a string containing the text disassembly of the given program
void disassemble_program(bytestream_t program, FILE* ofp);

#endif