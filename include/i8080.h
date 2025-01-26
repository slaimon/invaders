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

#define i8080_get_bc(m) \
    ((uint16_t) ((m)->B << 8) + (m)->C)

#define i8080_set_bc(m, v) { \
    (m)->B = (v) >> 8;       \
    (m)->C = (v) & 0xFF;     \
}

#define i8080_get_de(m) \
    ((uint16_t) ((m)->D << 8) + (m)->E)

#define i8080_set_de(m, v) { \
    (m)->D = (v) >> 8;       \
    (m)->E = (v) & 0xFF;     \
}

#define i8080_get_hl(m) \
    ((uint16_t) ((m)->H << 8) + (m)->L)

#define i8080_set_hl(m, v) { \
    (m)->H = (v) >> 8;       \
    (m)->L = (v) & 0xFF;     \
}

// Initialize a blank i8080 machine: all registers are zero
void i8080_init(i8080_t* machine);

// Read a contiguous block of memory "size" bytes long and starting at address "offset".
// Returns a pointer to the bytestream containing the data.
// The returned bytestream is only guaranteed to be "size" bytes long if offset + size <= I8080_MEMSIZE,
// otherwise it will only be I8080_MEMSIZE - offset bytes in length.
// Fails with exit() if the given machine was invalid.
bytestream_t* i8080_memory_read(const i8080_t* machine, const uint16_t size, const uint16_t offset);

// Copy the given block of data to the machine's memory at the specified offset.
// Fails with exit() if there is insufficient memory.
void i8080_memory_write(i8080_t* machine, const bytestream_t src, const uint16_t offset);

// Executes one instruction and returns one of the following values:
// - the number of cycles elapsed, if the execution was successful
// - I8080_HALT if the cpu has encountered a halt instruction
// - I8080_FAIL if the machine is invalid
int i8080_execute(i8080_t* machine);

// Executes an RST-type interrupt; the values accepted for "restart" are 0 through 7.
// Returns I8080_FAIL if the machine or the restart parameter is invalid, else returns
// the number of elapsed cycles
int i8080_interrupt(i8080_t* machine, uint8_t restart);

#endif