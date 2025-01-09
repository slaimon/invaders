#ifndef __I8080_H__
#define __I8080_H__


#include <stdint.h>
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

} i8080_state_t;


// initialize a blank i8080 state: all registers are zero except the Stack Pointer, which is set to
// point to the last addressable block of memory (I8080_MEMSIZE - 1)
void i8080_init(i8080_state_t* state);

// copy the given block of data to the state's memory at the specified offset.
// fails with exit() if there is insufficient memory.
void i8080_setMemory(i8080_state_t* state, bytestream_t src, uint16_t offset);

// executes one instruction and returns either the length in bytes of the executed instruction or
// one of the following values:
// - I8080_FAIL if the given state was invalid
// - I8080_HALT if the cpu has determined it should halt
//
// TODO: perché mai dovrebbe restituire la lunghezza dell'istruzione? non può semplicemente
//       incrementare il program counter? e a quel punto potrei fargli restituire un enum anziché un
//       int
int i8080_execute(i8080_state_t* state);


#endif