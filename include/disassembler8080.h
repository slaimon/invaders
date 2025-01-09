#ifndef __DISASSEMBLER8080_H__
#define __DISASSEMBLER8080_H__

#include "bytestream.h"
#include "instruction8080.h"

#define DISASSEMBLER8080_FAIL   -2  // retval on error
#define DISASSEMBLER8080_LITTLE_ENDIAN  true


// returns a struct containing the disassembly of the instruction found at the specified address in the memory 
instruction8080_t disassemble_instruction( uint8_t* mem , unsigned int addr );

// returns a string containing the text disassembly of the given program
void disassemble_program(bytestream_t program, FILE* ofp);

#endif