#ifndef __I8080_H__
#define __I8080_H__


#include <stdint.h>
#include <stdbool.h>
#include "bytestream.h"

#define I8080_MEMSIZE 0x10000

#define I8080_HALT    -1
#define I8080_FAIL    -2

//#define SUPPORT_CPM_CALLS

typedef struct {
    // interrupts flip-flop:
    uint8_t interrupts;
    
    // flags:
    unsigned int signFlag : 1;
    unsigned int zeroFlag : 1;
    unsigned int auxCarryFlag: 1;
    unsigned int parityFlag : 1;
    unsigned int carryFlag : 1;

    // registers:
    uint8_t A, B, C, D, E, H, L;
    
    // counters:
    uint16_t stackPointer;
    uint16_t programCounter;
    
    // memory:
    uint8_t mem[I8080_MEMSIZE];

} i8080_t;

typedef enum {
    I8080_REGISTER_A,
    I8080_REGISTER_B,
    I8080_REGISTER_C,
    I8080_REGISTER_D,
    I8080_REGISTER_E,
    I8080_REGISTER_H,
    I8080_REGISTER_L,
    I8080_REGISTER_BC,
    I8080_REGISTER_DE,
    I8080_REGISTER_HL,
    I8080_REGISTER_PROGRAM_COUNTER,
    I8080_REGISTER_STACK_POINTER
} i8080_register_t;

typedef enum {
    I8080_FLAG_SIGN,
    I8080_FLAG_ZERO,
    I8080_FLAG_CARRY,
    I8080_FLAG_AUXCARRY,
    I8080_FLAG_PARITY
} i8080_flag_t;


// Initialize a blank i8080 machine: all registers are zero except the Stack Pointer, which is set to
// point to the last addressable block of memory (I8080_MEMSIZE - 1)
void i8080_init(i8080_t* machine);

// Read the value of a CPU register.
uint16_t i8080_register_get(const i8080_t* machine, const i8080_register_t register);

// Set the value of a CPU register.
// If register is 8 bits wide, only the lower 8 bits of value will be used 
void i8080_register_set(i8080_t* machine, const i8080_register_t register, const uint16_t value);

// Read the value of a CPU flag.
bool i8080_flag_get(const i8080_t* machine, const i8080_flag_t flag);

// Set the value of a CPU flag.
// If register is 8 bits wide, only the lower 8 bits of value will be used 
void i8080_flag_set(i8080_t* machine, const i8080_flag_t flag, const bool value);

// Read a contiguous block of memory "size" bytes long and starting at address "offset".
// Returns a pointer to the bytestream containing the data.
// The returned bytestream is only guaranteed to be "size" bytes long if offset + size <= I8080_MEMSIZE,
// otherwise it will only be I8080_MEMSIZE - offset bytes in length.
// Fails with exit() if the given machine was invalid.
bytestream_t* i8080_memory_read(const i8080_t* machine, const uint16_t size, const uint16_t offset);

// Copy the given block of data to the machine's memory at the specified offset.
// Fails with exit() if there is insufficient memory.
void i8080_memory_write(i8080_t* machine, const bytestream_t src, const uint16_t offset);

// Executes one instruction and returns either the length in bytes of the executed instruction or
// one of the following values:
// - I8080_FAIL if the given machine was invalid
// - I8080_HALT if the cpu has determined it should halt
//
// TODO: perché mai dovrebbe restituire la lunghezza dell'istruzione? non può semplicemente
//       incrementare il program counter? e a quel punto potrei fargli restituire un enum anziché un
//       int
int i8080_execute(i8080_t* machine);


#endif