#ifndef __CPU8080_H__
#define __CPU8080_H__


#include <stdint.h>

#define CPU8080_MEMSIZE 0x10000

#define CPU8080_HALT    -1
#define CPU8080_FAIL    -2

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
    uint8_t *mem;

} cpu8080_state_t;

// executes one instruction and returns either the length in bytes of the executed instruction or one of the following values:
// CPU8080_FAIL if the given state was invalid
// CPU8080_HALT if the cpu has determined it should halt
//
// TODO: perché mai dovrebbe restituire la lunghezza dell'istruzione? non può semplicemente incrementare il program counter?
//       e a quel punto potrei fargli restituire un enum anziché un int
int cpu8080_execute( cpu8080_state_t* state );


#endif