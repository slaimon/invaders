#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "../include/i8080.h"

/* ------------- BYTE PARITY LOOKUP TABLE ----------- */

#define P2(n) n, n ^ 1, n ^ 1, n
#define P4(n) P2(n), P2(n ^ 1), P2(n ^ 1), P2(n)
#define P6(n) P4(n), P4(n ^ 1), P4(n ^ 1), P4(n)
#define LOOK_UP P6(0), P6(1), P6(1), P6(0)

/* ------------- MACRO UTILS: ----------------------- */

// CALL direct address or register
#define CALL_DIRECT(x)                                                              \
            machine->programCounter += 3;                                           \
            machine->mem[machine->stackPointer-1] = machine->programCounter >> 8;   \
            machine->mem[machine->stackPointer-2] = machine->programCounter & 0xFF; \
            machine->stackPointer -= 2;                                             \
            machine->programCounter = (x);                                          \
            instructionLength = 0;
                
// RET after termination of a subroutine
#define RETURN                                                                       \
            memoryAddressRegister = machine->stackPointer;                           \
            machine->programCounter = (machine->mem[memoryAddressRegister+1] << 8) + \
                                     machine->mem[memoryAddressRegister];            \
            machine->stackPointer += 2;                                              \
            instructionLength = 0;

// SBB - subtract with borrow
#define SBB(x)                                                          \
            tmp1 = machine->A - ((x) + machine->carryFlag);             \
            tmp2 = machine->A ^ ( -((x) + machine->carryFlag) + 1);     \
            if( (tmp1 & 0xFF00) != 0)                                   \
                machine->carryFlag = 0;                                 \
            else                                                        \
                machine->carryFlag = 1;                                 \
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )                    \
                machine->auxCarryFlag = 1;                              \
            else                                                        \
                machine->auxCarryFlag = 0;                              \
            machine->A = tmp1;

/* ------------ FLAG UPDATE MACROS ----------------*/

#define SIGN(x) \
            machine->signFlag = ((x) >> 7) ;

#define ZERO(x) \
            machine->zeroFlag = ( (x) == 0 ) ? (1) : (0) ;

#define PARITY(x) \
            machine->parityFlag = ~ table[(x) & 0xff];

const uint8_t table[256] = { LOOK_UP };


void i8080_init(i8080_t* machine) {	
    // set all to zero except stack pointer
    memset(machine, 0, sizeof(i8080_t));
	machine->stackPointer = I8080_MEMSIZE-1;
}

bytestream_t* i8080_memory_read(const i8080_t* machine, const uint16_t size, const uint16_t offset) {
    uint16_t true_size = (offset + size > I8080_MEMSIZE) ?
                            (I8080_MEMSIZE - offset) :
                            size;
    bytestream_t* dest = bytestream_new(true_size);

    memcpy(dest->data, &machine->mem[offset], dest->size);
    return dest;
}

void i8080_memory_write(i8080_t* machine, const bytestream_t src, uint16_t offset) {
    if (src.size + offset > I8080_MEMSIZE) {
        fprintf(stderr, "ERROR: i8080_memory_write: not enough memory.\n");
        exit(EXIT_FAILURE);
    }

    memcpy(&machine->mem[offset], src.data, src.size);
}

int i8080_execute(i8080_t* machine ) {
    bool haltSignal = false;
    
    if ( machine == NULL )
        return I8080_FAIL;
    
    const uint16_t currentProgramCounter = machine->programCounter;
    const uint8_t instruction = machine->mem[currentProgramCounter];
    uint16_t memoryAddressRegister = (machine->H << 8) + machine->L;  // memory address "virtual register" [HL]

    uint8_t instructionLength = 1;  // default value
    uint16_t tmp1, tmp2;
    
    switch (instruction) {
        case 0x00: {
            // NOP
            
            break;
        }
        case 0x01: {
            // LXI BC: load immediate to register pair BC
            
            machine->B = machine->mem[currentProgramCounter+2];
            machine->C = machine->mem[currentProgramCounter+1];
            
            instructionLength = 3;
            break;
        }
        case 0x02: {
            // STAX BC: store A through register pair BC
            memoryAddressRegister = (machine->B << 8) + machine->C;
            machine->mem[memoryAddressRegister] = machine->A;
            
            break;
        }
        case 0x03: {
            // INX BC

            if ( machine->C == 0xFF ) {
                machine->B = (uint8_t) machine->B + 1;
                machine->C = 0;
            }
            else machine->C = (uint8_t) machine->C + 1;
            
            break;
        }
        case 0x04: {
            // INR B
            
            tmp1 = machine->B & 0xF0;
            machine->B = 1 + (uint8_t) machine->B;
            
            if( ( machine->B & 0xF0 ) != tmp1 )
                machine->auxCarryFlag = 1;
                else
                machine->auxCarryFlag = 0;
            
            SIGN(machine->B)
            ZERO(machine->B)
            PARITY(machine->B)
            
            break;
        }
        case 0x05: {
            // DCR B
            tmp1 = machine->B & 0xF0;
            machine->B = (uint8_t) machine->B - 1;
            
            if( ( machine->B & 0xF0 ) != tmp1 )
                machine->auxCarryFlag = 1;
                else
                machine->auxCarryFlag = 0;
            
            SIGN(machine->B)
            ZERO(machine->B)
            PARITY(machine->B)
            
            break;
        }
        case 0x06: {
            // MVI B
            machine->B = machine->mem[currentProgramCounter+1];
            
            instructionLength = 2;
            break;
        }
        case 0x07: {
            // RLC A : rotate left
            
            machine->carryFlag = machine->A >> 7;
            machine->A = machine->A << 1;
            machine->A += machine->carryFlag;
            
            break;
        }
        case 0x08: {
            // NOP
            
            break;
        }
        case 0x09: {
            // DAD BC: add register pair to HL
            
            tmp1 = machine->L + machine->C;
            tmp2 = machine->H + machine->B + (tmp1 >> 8);
            
            machine->L = tmp1 & 255;
            machine->H = tmp2 & 255;
            
            if((tmp2 >> 8) != 0)
                machine->carryFlag = 1;
                else
                machine->carryFlag = 0;
            
            break;
        }
        case 0x0A: {
            // LDAX BC : load A through BC
            tmp1 = (machine->B << 8) + machine->C;
            machine->A = machine->mem[tmp1];
            
            break;
        }
        case 0x0B: {
            // DCX BC
            
            if ( machine->C == 0x00 ) {
                machine->B = (uint8_t) machine->B - 1;
                machine->C = 0xFF;
            }
            else machine->C = (uint8_t) machine->C - 1;
            
            break;
        }
        case 0x0C: {
            // INR C
            tmp1 = machine->C & 0xF0;
            machine->C = (uint8_t) machine->C + 1;
            
            if( ( machine->C & 0xF0 ) != tmp1 )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            SIGN(machine->C)
            ZERO(machine->C)
            PARITY(machine->C)
            
            break;
        }
        case 0x0D: {
            // DCR C
            tmp1 = machine->C & 0xF0;
            machine->C = (uint8_t) machine->C - 1;
            
            if( ( machine->C & 0xF0 ) != tmp1 )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            SIGN(machine->C)
            ZERO(machine->C)
            PARITY(machine->C)
            
            break;
        }
        case 0x0E: {
            // MVI C
            machine->C = machine->mem[currentProgramCounter+1];
            
            instructionLength = 2;
            break;
        }
        case 0x0F: {
            // RRC A : rotate right
            
            machine->carryFlag = machine->A & 1;
            machine->A = machine->A >> 1;
            machine->A += machine->carryFlag << 7;
            
            instructionLength = 1;
            break;
        }
        case 0x10: {
            // NOP
            
            break;
        }
        case 0x11: {
            // LXI DE : load immediate to register pair
            
            machine->D = machine->mem[currentProgramCounter+2];
            machine->E = machine->mem[currentProgramCounter+1];
            
            instructionLength = 3;
            break;
        }
        case 0x12: {
            // STAX DE : store accumulator A through register pair DE
            memoryAddressRegister = (machine->D << 8) + machine->E;
            machine->mem[memoryAddressRegister] = machine->A;
            
            break;
        }
        case 0x13: {
            // INX DE
            
            if ( machine->E == 0xFF ) {
                machine->D = 1 + (uint8_t) machine->D;
                machine->E = 0;
            }
            else
                machine->E = 1 + (uint8_t) machine->E;
            
            break;
        }
        case 0x14: {
            // INR D
            tmp1 = machine->D & 0xF0;
            machine->D = (uint8_t) machine->D + 1;
            
            if( ( machine->D & 0xF0 ) != tmp1 )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            SIGN(machine->D)
            ZERO(machine->D)
            PARITY(machine->D)
            
            break;
        }
        case 0x15: {
            // DCR D
            tmp1 = machine->D & 0xF0;
            machine->D = (uint8_t) machine->D - 1;
            
            if( (machine->D & 0xF0 ) != tmp1 )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            SIGN(machine->D)
            ZERO(machine->D)
            PARITY(machine->D)
            
            break;
        }
        case 0x16: {
            // MVI D
            machine->D = machine->mem[currentProgramCounter+1];
            
            instructionLength = 2;
            break;
        }
        case 0x17: {
            // RAL : rotate A through carry bit
            
            tmp1 = machine->carryFlag;
            machine->carryFlag = machine->A >> 7;
            machine->A = machine->A << 1;
            machine->A += tmp1;
            
            break;
        }
        case 0x18: {
            // NOP
            
            break;
        }
        case 0x19: {
            // DAD DE
            tmp1 = machine->L + machine->E;
            tmp2 = machine->H + machine->D + (tmp1 >> 8);
            
            machine->L = tmp1 & 255;
            machine->H = tmp2 & 255;
            
            if( ( tmp2 >> 8 ) != 0 )
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;
                
            break;
        }
        case 0x1A: {
            // LDAX DE : load A through DE
            tmp1 = (machine->D << 8) + machine->E;
            machine->A = machine->mem[tmp1];
            
            break;
        }
        case 0x1B: {
            // DCX DE

            if ( machine->E == 0x00 ) {
                machine->D = (uint8_t) machine->D - 1;
                machine->E = 0xFF;
            }
            else
                machine->E = (uint8_t) machine->E - 1;
            
            break;
        }
        case 0x1C: {
            // INR E
            tmp1 = machine->E & 0xF0;
            machine->E = (uint8_t) machine->E + 1;
            
            if( ( machine->E & 0xF0 ) != tmp1 )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            SIGN(machine->E)
            ZERO(machine->E)
            PARITY(machine->E)
            
            break;
        }
        case 0x1D: {
            // DCR E
            tmp1 = machine->E & 0xF0;
            machine->E = (uint8_t) machine->E - 1;
            
            if( ( machine->E & 0xF0 ) != tmp1 )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            SIGN(machine->E)
            ZERO(machine->E)
            PARITY(machine->E)
            
            break;
        }
        case 0x1E: {
            // MVI E
            
            machine->E = machine->mem[currentProgramCounter+1];
            
            instructionLength = 2;
            break;
        }
        case 0x1F: {
            // RAR : rotate right through carry
            
            tmp1         = machine->carryFlag;
            machine->carryFlag = machine->A & 1;
            machine->A = machine->A >> 1;
            machine->A += tmp1 << 7;
            
            break;
        }
        case 0x20: {
            // NOP
            
            break;
        }
        case 0x21: {
            // LXI HL : load immediate to register pair HL
            
            machine->L = machine->mem[currentProgramCounter+1];
            machine->H = machine->mem[currentProgramCounter+2];
            
            instructionLength = 3;
            break;
        }
        case 0x22: {
            // SHLD: store HL direct
            memoryAddressRegister = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
            machine->mem[memoryAddressRegister] = machine->L;
            machine->mem[memoryAddressRegister+1] = machine->H;
            
            instructionLength = 3;
            break;
        }
        case 0x23: {
            // INX HL
            
            if ( machine->L == 0xFF ) {
                machine->H = (uint8_t) machine->H + 1;
                machine->L = 0;
            }
            else
                machine->L = (uint8_t) machine->L + 1;
            
            
            break;
        }
        case 0x24: {
            // INR H
            tmp1 = machine->H & 0xF0;
            machine->H = (uint8_t) machine->H + 1;
            
            if( ( machine->H & 0xF0 ) != tmp1 )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            SIGN(machine->H)
            ZERO(machine->H)
            PARITY(machine->H)
            
            break;
        }
        case 0x25: {
            // DCR H
            tmp1 = machine->H & 0xF0;
            machine->H = (uint8_t) machine->H - 1;
            
            if( ( machine->H & 0xF0 ) != tmp1 )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            SIGN(machine->H)
            ZERO(machine->H)
            PARITY(machine->H)
            
            break;
        }
        case 0x26: {
            // MVI H
            machine->H = machine->mem[currentProgramCounter+1];
            
            instructionLength = 2;
            break;
        }
        case 0x27: {
            // DAA - Decimal Adjust Accumulator : SZAPC
            
            tmp1 = machine->A & 0xF0;
            
            if( (machine->A & 0x0F) > 9 || machine->auxCarryFlag == 1)
                machine->A += 6;
            if( (machine->A & 0xF0) != tmp1)
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
                    
            tmp1 = machine->A;
            
            if( ( (machine->A & 0xF0) >> 4 ) > 9 || machine->carryFlag == 1)
                tmp1 += 0x60;
            if( tmp1 >> 8 != 0 )
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;
            
            machine->A = tmp1 & 0xFF;
            
            SIGN(machine->A)
            ZERO(machine->A)
            PARITY(machine->A)
            
            break;
        }
        case 0x28: {
            // NOP
            
            break;
        }
        case 0x29: {
            // DAD HL - add register pair to HL
            
            tmp1 = machine->L + machine->L;
            tmp2 = machine->H + machine->H + (tmp1 >> 8);
            
            machine->L = tmp1 & 255;
            machine->H = tmp2 & 255;
            
            if((tmp2 >> 8) != 0)
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;

            break;
        }
        case 0x2A: {
            // LHLD HL - load HL direct
    
            memoryAddressRegister = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
            machine->H = machine->mem[memoryAddressRegister+1];
            machine->L = machine->mem[memoryAddressRegister];
            
            instructionLength = 3;
            break;
        }
        case 0x2B: {
            // DCX HL
            
            if ( machine->L == 0x00 ) {
                machine->H = (uint8_t) machine->H - 1;
                machine->L = 0xFF;
            }
            else
                machine->L = (uint8_t) machine->L - 1;
            
            break;
        }
        case 0x2C: {
            // INR L
            tmp1 = machine->L & 0xF0;
            machine->L = (uint8_t) machine->L + 1;
            
            if( (machine->L & 0xF0 ) != tmp1 )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            SIGN(machine->L)
            ZERO(machine->L)
            PARITY(machine->L)
            
            break;
        }
        case 0x2D: {
            // DCR L
            tmp1 = machine->L & 0xF0;
            machine->L = (uint8_t) machine->L - 1;
            
            if( ( machine->L & 0xF0 ) != tmp1 )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            SIGN(machine->L)
            ZERO(machine->L)
            PARITY(machine->L)
            
            break;
        }
        case 0x2E: {
            // MVI L
            machine->L = machine->mem[currentProgramCounter+1];
            
            instructionLength = 2;
            break;
        }
        case 0x2F: {
            // CMA
            machine->A = ~ machine->A;
            
            break;
        }
        case 0x30: {
            // NOP
            
            break;
        }
        case 0x31: {
            // LXI SP
            machine->stackPointer = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
            
            instructionLength = 3;
            break;
        }
        case 0x32: {
            // STA - store A direct
            
            memoryAddressRegister = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
            machine->mem[memoryAddressRegister] = machine->A;
            
            instructionLength = 3;
            break;
        }
        case 0x33: {
            // INX SP
            
            machine->stackPointer = (uint16_t) machine->stackPointer + 1;
            
            break;
        }
        case 0x34: {
            // INR M
            tmp1 = machine->mem[memoryAddressRegister] & 0xF0;
            machine->mem[memoryAddressRegister] = (uint8_t) machine->mem[memoryAddressRegister] + 1;
            
            if( ( machine->mem[memoryAddressRegister] & 0xF0 ) != tmp1 )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            SIGN(machine->mem[memoryAddressRegister])
            ZERO(machine->mem[memoryAddressRegister])
            PARITY(machine->mem[memoryAddressRegister])
            
            break;
        }
        case 0x35: {
            // DCR M
            tmp1 = machine->mem[memoryAddressRegister] & 0xF0;
            machine->mem[memoryAddressRegister] = (uint8_t) machine->mem[memoryAddressRegister] - 1;
            
            if( ( machine->mem[memoryAddressRegister] & 0xF0 ) != tmp1 )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            SIGN(machine->mem[memoryAddressRegister])
            ZERO(machine->mem[memoryAddressRegister])
            PARITY(machine->mem[memoryAddressRegister])
            
            break;
        }
        case 0x36: {
            // MVI M
            machine->mem[memoryAddressRegister] = machine->mem[currentProgramCounter + 1];
            
            instructionLength = 2;
            break;
        }
        case 0x37: {
            // STC
            machine->carryFlag = 1;
            
            break;
        }
        case 0x38: {
            // NOP
            
            break;
        }
        case 0x39: {
            // DAD SP - add register pair to HL
            
            tmp1 = machine->L + (machine->stackPointer & 0x00FF);
            tmp2 = machine->H + (machine->stackPointer & 0xFF00) + (tmp1 >> 8);
            
            machine->L = tmp1 & 255;
            machine->H = tmp2 & 255;
            
            if( (tmp2 >> 8) != 0)
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;

            break;
        }
        case 0x3A: {
            // LDA: load A direct
            memoryAddressRegister = (machine->mem[currentProgramCounter + 2] << 8) + machine->mem[currentProgramCounter + 1];
            machine->A = machine->mem[memoryAddressRegister];
            
            instructionLength = 3;
            break;
        }
        case 0x3B: {
            // DCX SP
            
            machine->stackPointer = (uint16_t) machine->stackPointer - 1;
            
            break;
        }
        case 0x3C: {
            // INR A
            tmp1 = machine->A & 0xF0;
            machine->A = (uint8_t) machine->A + 1;
            
            if( ( machine->A & 0xF0 ) != tmp1 )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            SIGN(machine->A)
            ZERO(machine->A)
            PARITY(machine->A)
            
            break;
        }
        case 0x3D: {
            // DCR A
            tmp1 = machine->A & 0xF0;
            machine->A = (uint8_t) machine->A - 1;
            
            if( ( machine->A & 0xF0 ) != tmp1 )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            SIGN(machine->A)
            ZERO(machine->A)
            PARITY(machine->A)
            
            break;
        }
        case 0x3E: {
            // MVI A
            machine->A = machine->mem[currentProgramCounter + 1];
            
            instructionLength = 2;
            break;
        }
        case 0x3F: {
            // CMC (Complement Carry)
            
            machine->carryFlag = (machine->carryFlag == 0) ? (1) : (0);
            
            break;
        }
        case 0x40: {
            // MOV B,B
            
            break;
        }
        case 0x41: {
            // MOV B,C
            machine->B = machine->C;
            
            break;
        }
        case 0x42: {
            // MOV B,D
            machine->B = machine->D;
            
            break;
        }
        case 0x43: {
            // MOV B,E
            machine->B = machine->E;
            
            break;
        }
        case 0x44: {
            // MOV B,H
            machine->B = machine->H;
            
            break;
        }
        case 0x45: {
            // MOV B,L
            machine->B = machine->L;
            
            break;
        }
        case 0x46: {
            // MOV B,M
            machine->B = machine->mem[memoryAddressRegister];
            
            break;
        }
        case 0x47: {
            // MOV B,A
            machine->B = machine->A;
            
            break;
        }
        case 0x48: {
            // MOV C,B
            machine->C = machine->B;
            
            break;
        }
        case 0x49: {
            // MOV C,C
            
            break;
        }
        case 0x4A: {
            // MOV C,D
            machine->C = machine->D;
            
            break;
        }
        case 0x4B: {
            // MOV C,E
            machine->C = machine->E;
            
            break;
        }
        case 0x4C: {
            // MOV C,H
            machine->C = machine->H;
            
            break;
        }
        case 0x4D: {
            // MOV C,L
            machine->C = machine->L;
            
            break;
        }
        case 0x4E: {
            // MOV C,M
            machine->C = machine->mem[memoryAddressRegister];
            
            break;
        }
        case 0x4F: {
            // MOV C,A
            machine->C = machine->A;
            
            break;
        }
        case 0x50: {
            // MOV D,B
            machine->D = machine->B;
            
            break;
        }
        case 0x51: {
            // MOV D,C
            machine->D = machine->C;
            
            break;
        }
        case 0x52: {
            // MOV D,D
            
            break;
        }
        case 0x53: {
            // MOV D,E
            machine->D = machine->E;
            
            break;
        }
        case 0x54: {
            // MOV D,H
            machine->D = machine->H;
            
            break;
        }
        case 0x55: {
            // MOV D,L
            machine->D = machine->L;
            
            break;
        }
        case 0x56: {
            // MOV D,M
            machine->D = machine->mem[memoryAddressRegister];
            
            break;
        }
        case 0x57: {
            // MOV D,A
            machine->D = machine->A;
            break;
        }
        case 0x58: {
            // MOV E,B
            machine->E = machine->B;
            
            break;
        }
        case 0x59: {
            // MOV E,C
            machine->E = machine->C;
            
            break;
        }
        case 0x5A: {
            // MOV E,D
            machine->E = machine->D;
            
            break;
        }
        case 0x5B: {
            // MOV E,E
            
            break;
        }
        case 0x5C: {
            // MOV E,H
            machine->E = machine->H;
            
            break;
        }
        case 0x5D: {
            // MOV E,L
            machine->E = machine->L;
            
            break;
        }
        case 0x5E: {
            // MOV E,M
            machine->E = machine->mem[memoryAddressRegister];
            
            break;
        }
        case 0x5F: {
            // MOV E,A
            machine->E = machine->A;
            
            break;
        }
        case 0x60: {
            // MOV H,B
            machine->H = machine->B;
            
            break;
        }
        case 0x61: {
            // MOV H,C
            machine->H = machine->C;
            
            break;
        }
        case 0x62: {
            // MOV H,D
            machine->H = machine->D;
            
            break;
        }
        case 0x63: {
            // MOV H,E
            machine->H = machine->E;
            
            break;
        }
        case 0x64: {
            // MOV H,H
            
            break;
        }
        case 0x65: {
            // MOV H,L
            machine->H = machine->L;
            
            break;
        }
        case 0x66: {
            // MOV H,M
            machine->H = machine->mem[memoryAddressRegister];
            
            break;
        }
        case 0x67: {
            // MOV H,A
            machine->H = machine->A;
            
            break;
        }
        case 0x68: {
            // MOV L,B
            machine->L = machine->B;
            
            break;
        }
        case 0x69: {
            // MOV L,C
            machine->L = machine->C;
            
            break;
        }
        case 0x6A: {
            // MOV L,D
            machine->L = machine->D;
            
            break;
        }
        case 0x6B: {
            // MOV L,E
            machine->L = machine->E;
            
            break;
        }
        case 0x6C: {
            // MOV L,H
            machine->L = machine->H;
            
            break;
        }
        case 0x6D: {
            // MOV L,L
            
            break;
        }
        case 0x6E: {
            // MOV L,M
            machine->L = machine->mem[memoryAddressRegister];
            
            break;
        }
        case 0x6F: {
            // MOV L,A
            machine->L = machine->A;
            
            break;
        }
        case 0x70: {
            // MOV M,B
            machine->mem[memoryAddressRegister] = machine->B;
            
            break;
        }
        case 0x71: {
            // MOV M,C
            machine->mem[memoryAddressRegister] = machine->C;
            
            break;
        }
        case 0x72: {
            // MOV M,D
            machine->mem[memoryAddressRegister] = machine->D;
            
            break;
        }
        case 0x73: {
            // MOV M,E
            machine->mem[memoryAddressRegister] = machine->E;
            
            break;
        }
        case 0x74: {
            // MOV M,H
            machine->mem[memoryAddressRegister] = machine->H;
            
            break;
        }
        case 0x75: {
            // MOV M,L
            machine->mem[memoryAddressRegister] = machine->L;
            
            break;
        }
        case 0x76: {
            // HLT
            
            haltSignal = true;
            
            break;
        }
        case 0x77: {
            // MOV M,A
            machine->mem[memoryAddressRegister] = machine->A;
            
            break;
        }
        case 0x78: {
            // MOV A,B
            machine->A = machine->B;
            
            break;
        }
        case 0x79: {
            // MOV A,C
            machine->A = machine->C;
            
            break;
        }
        case 0x7A: {
            // MOV A,D
            machine->A = machine->D;
            
            break;
        }
        case 0x7B: {
            // MOV A,E
            machine->A = machine->E;
            
            break;
        }
        case 0x7C: {
            // MOV A,H
            machine->A = machine->H;
            
            break;
        }
        case 0x7D: {
            // MOV A,L
            machine->A = machine->L;
            
            break;
        }
        case 0x7E: {
            // MOV A,M
            machine->A = machine->mem[memoryAddressRegister];
            
            break;
        }
        case 0x7F: {
            // MOV A,A
            
            break;
        }
        case 0x80: {
            // ADD B
            tmp1 = machine->A + machine->B;
            tmp2 = machine->A ^ machine->B;
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
                
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x81: {
            // ADD C
            tmp1 = machine->A + machine->C;
            tmp2 = machine->A ^ machine->C;
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x82: {
            // ADD D
            tmp1 = machine->A + machine->D;
            tmp2 = machine->A ^ machine->D;
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x83: {
            // ADD E
            tmp1 = machine->A + machine->E;
            tmp2 = machine->A ^ machine->E;
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x84: {
            // ADD H
            tmp1 = machine->A + machine->H;
            tmp2 = machine->A ^ machine->H;
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x85: {
            // ADD L
            tmp1 = machine->A + machine->L;
            tmp2 = machine->A ^ machine->L;
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x86: {
            // ADD M
            tmp1 = machine->A + machine->mem[memoryAddressRegister];
            tmp2 = machine->A ^ machine->mem[memoryAddressRegister];
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x87: {
            // ADD A
            tmp1 = machine->A + machine->A;
            tmp2 = machine->A ^ machine->A;
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x88: {
            //    ADC B
            tmp1 = machine->A + machine->B + machine->carryFlag;
            tmp2 = machine->A ^ (machine->B + machine->carryFlag);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
                
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x89: {
            //    ADC C
            tmp1 = machine->A + machine->C + machine->carryFlag;
            tmp2 = machine->A ^ (machine->C + machine->carryFlag);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x8A: {
            //    ADC D
            tmp1 = machine->A + machine->D + machine->carryFlag;
            tmp2 = machine->A ^ (machine->D + machine->carryFlag);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x8B: {
            // ADC E
            tmp1 = machine->A + machine->E + machine->carryFlag;
            tmp2 = machine->A ^ (machine->E + machine->carryFlag);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x8C: {
            // ADC H
            tmp1 = machine->A + machine->H + machine->carryFlag;
            tmp2 = machine->A ^ (machine->H + machine->carryFlag);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x8D: {
            // ADC L
            tmp1 = machine->A + machine->L + machine->carryFlag;
            tmp2 = machine->A ^ (machine->L + machine->carryFlag);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x8E: {
            // ADC M
            tmp1 = machine->A + machine->mem[memoryAddressRegister] + machine->carryFlag;
            tmp2 = machine->A ^ (machine->mem[memoryAddressRegister] + machine->carryFlag);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x8F: {
            // ADC A
            tmp1 = machine->A + machine->A + machine->carryFlag;
            tmp2 = machine->A ^ (machine->A + machine->carryFlag);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x90: {
            // SUB B
            tmp1 = machine->A - machine->B;
            tmp2 = machine->A ^ (- machine->B + 1);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 0;
            else
                machine->carryFlag = 1;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x91: {
            // SUB C
            tmp1 = machine->A - machine->C;
            tmp2 = machine->A ^ (- machine->C + 1);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 0;
            else
                machine->carryFlag = 1;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x92: {
            // SUB D
            tmp1 = machine->A - machine->D;
            tmp2 = machine->A ^ (- machine->D + 1);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 0;
            else
                machine->carryFlag = 1;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x93: {
            // SUB E
            tmp1 = machine->A - machine->E;
            tmp2 = machine->A ^ (- machine->E + 1);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 0;
            else
                machine->carryFlag = 1;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x94: {
            // SUB H
            tmp1 = machine->A - machine->H;
            tmp2 = machine->A ^ (- machine->H + 1);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 0;
            else
                machine->carryFlag = 1;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x95: {
            // SUB L
            tmp1 = machine->A - machine->L;
            tmp2 = machine->A ^ (- machine->L + 1);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 0;
            else
                machine->carryFlag = 1;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x96: {
            // SUB M
            tmp1 = machine->A - machine->mem[memoryAddressRegister];
            tmp2 = machine->A ^ (- machine->mem[memoryAddressRegister] + 1);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 0;
            else
                machine->carryFlag = 1;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x97: {
            // SUB A
            tmp1 = machine->A - machine->A;
            tmp2 = machine->A ^ (- machine->A + 1);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 0;
            else
                machine->carryFlag = 1;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x98: { // SBB - subtract with borrow
            // SBB B
            tmp1 = machine->A - (machine->B + machine->carryFlag);
            tmp2 = machine->A ^ ( -(machine->B + machine->carryFlag) + 1);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 0;
            else
                machine->carryFlag = 1;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x99: {
            // SBB C
            tmp1 = machine->A - (machine->C + machine->carryFlag);
            tmp2 = machine->A ^ ( -(machine->C + machine->carryFlag) + 1);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 0;
            else
                machine->carryFlag = 1;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x9A: {
            // SBB D
            tmp1 = machine->A - (machine->D + machine->carryFlag);
            tmp2 = machine->A ^ ( -(machine->D + machine->carryFlag) + 1);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 0;
            else
                machine->carryFlag = 1;

            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x9B: {
            // SBB E
            tmp1 = machine->A - (machine->E + machine->carryFlag);
            tmp2 = machine->A ^ ( -(machine->E + machine->carryFlag) + 1);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 0;
            else
                machine->carryFlag = 1;
            
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x9C: {
            // SBB H
            tmp1 = machine->A - (machine->H + machine->carryFlag);
            tmp2 = machine->A ^ ( -(machine->H + machine->carryFlag) + 1);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 0;
            else
                machine->carryFlag = 1;
            
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x9D: {
            // SBB L
            tmp1 = machine->A - (machine->L + machine->carryFlag);
            tmp2 = machine->A ^ ( -(machine->L + machine->carryFlag) + 1);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 0;
            else
                machine->carryFlag = 1;
            
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x9E: {
            // SBB M
            tmp1 = machine->A - (machine->mem[memoryAddressRegister] + machine->carryFlag);
            tmp2 = machine->A ^ ( -(machine->mem[memoryAddressRegister] + machine->carryFlag) + 1);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 0;
            else
                machine->carryFlag = 1;
            
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0x9F: {
            // SBB A
            tmp1 = machine->A - (machine->A + machine->carryFlag);
            tmp2 = machine->A ^ ( -(machine->A + machine->carryFlag) + 1);
            
            if( (tmp1 & 0xFF00) != 0)
                machine->carryFlag = 0;
            else
                machine->carryFlag = 1;
            
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xA0: {    // for AND R instructions: AUX.C = ( (A & 8) | (R & 8) )
            // ANA B
            tmp1 = machine->A & machine->B;
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = (machine->A & 8) | (machine->B & 8);
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xA1: {
            // ANA C
            tmp1 = machine->A & machine->C;
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = (machine->A & 8) | (machine->C & 8);
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xA2: {
            // ANA D
            tmp1 = machine->A & machine->D;
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = (machine->A & 8) | (machine->D & 8);
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xA3: {
            // ANA E
            tmp1 = machine->A & machine->E;
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = (machine->A & 8) | (machine->E & 8);
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xA4: {
            // ANA H
            tmp1 = machine->A & machine->H;
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = (machine->A & 8) | (machine->H & 8);
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xA5: {
            // ANA L
            tmp1 = machine->A & machine->L;
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = (machine->A & 8) | (machine->L & 8);
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xA6: {
            // ANA M
            tmp1 = machine->A & machine->mem[memoryAddressRegister];
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = (machine->A & 8) | (machine->mem[memoryAddressRegister] & 8);
            machine->A = tmp1;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xA7: {
            // ANA A
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = machine->A & 8;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xA8: {
            // XOR B
            machine->A = machine->A ^ machine->B;
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = 0;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xA9: {
            // XOR C
            machine->A = machine->A ^ machine->C;
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = 0;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xAA: {
            // XOR D
            machine->A = machine->A ^ machine->D;
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = 0;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xAB: {
            // XOR E
            machine->A = machine->A ^ machine->E;
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = 0;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xAC: {
            // XOR H
            machine->A = machine->A ^ machine->H;
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = 0;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xAD: {
            // XOR L
            machine->A = machine->A ^ machine->L;
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = 0;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xAE: {
            // XOR M
            machine->A = machine->A ^ machine->mem[memoryAddressRegister];
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = 0;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xAF: {
            // XOR A
            machine->A = 0;
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = 0;
            
            PARITY(machine->A)
            SIGN(machine->A)
            machine->zeroFlag = 1;
            
            break;
        }
        case 0xB0: {
            // OR B
            machine->A = machine->A | machine->B;
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = 0;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xB1: {
            // OR C
            machine->A = machine->A | machine->C;
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = 0;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xB2: {
            // OR D
            machine->A = machine->A | machine->D;
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = 0;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xB3: {
            // OR E
            machine->A = machine->A | machine->E;
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = 0;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xB4: {
            // OR H
            machine->A = machine->A | machine->H;
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = 0;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xB5: {
            // OR L
            machine->A = machine->A | machine->L;
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = 0;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xB6: {
            // OR M
            machine->A = machine->A | machine->mem[memoryAddressRegister];
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = 0;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xB7: {
            // OR A
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = 0;
            
            PARITY(machine->A)
            SIGN(machine->A)
            ZERO(machine->A)
            
            break;
        }
        case 0xB8: {
            // CMP B
            tmp1 = machine->A - machine->B;
                
            if ( (uint8_t) machine->A < (uint8_t) machine->B )        // geniale!
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;
                
            if ( (uint8_t) (machine->A & 0xF) < (uint8_t) (machine->B & 0xF))
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            /*
            if ( (machine->A & 128) == ~ (machine->B & 128) )
                machine->carryFlag = (tmp1 >> 8) & 1;
                else
                machine->carryFlag = ~ ((tmp1 >> 8) & 1);
                
            if ( (machine->A & 0x0F) - (machine->B & 0x0F) < 0 )
                machine->auxCarryFlag = 1;
                else
                machine->auxCarryFlag = 0;
            */
            PARITY(tmp1)
            SIGN(tmp1)
            ZERO(tmp1)
            
            break;
        }
        case 0xB9: {
            // CMP C
            tmp1 = machine->A - machine->C;
                
            if ( (uint8_t) machine->A < (uint8_t) machine->C )
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;
                
            if ( (uint8_t) (machine->A & 0xF) < (uint8_t) (machine->C & 0xF))
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            PARITY(tmp1)
            SIGN(tmp1)
            ZERO(tmp1)
            
            break;
        }
        case 0xBA: {
            // CMP D
            tmp1 = machine->A - machine->D;
                
            if ( (uint8_t) machine->A < (uint8_t) machine->D )
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;
            
            if ( (uint8_t) (machine->A & 0xF) < (uint8_t) (machine->D & 0xF))
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            PARITY(tmp1)
            SIGN(tmp1)
            ZERO(tmp1)
            
            break;
        }
        case 0xBB: {
            // CMP E
            tmp1 = machine->A - machine->E;
                
            if ( (uint8_t) machine->A < (uint8_t) machine->E )
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;
            
            if ( (uint8_t) (machine->A & 0xF) < (uint8_t) (machine->E & 0xF))
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            PARITY(tmp1)
            SIGN(tmp1)
            ZERO(tmp1)
            
            break;
        }
        case 0xBC: {
            // CMP H
            tmp1 = machine->A - machine->H;
                
            if ( (uint8_t) machine->A < (uint8_t) machine->H )
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;
            
            if ( (uint8_t) (machine->A & 0xF) < (uint8_t) (machine->H & 0xF))
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            PARITY(tmp1)
            SIGN(tmp1)
            ZERO(tmp1)
            
            break;
        }
        case 0xBD: {
            // CMP L
            tmp1 = machine->A - machine->L;
                
            if ( (uint8_t) machine->A < (uint8_t) machine->L )
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;
            
            if ( (uint8_t) (machine->A & 0xF) < (uint8_t) (machine->L & 0xF))
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            PARITY(tmp1)
            SIGN(tmp1)
            ZERO(tmp1)
            
            break;
        }
        case 0xBE: {
            // CMP M
            tmp1 = machine->A - machine->mem[memoryAddressRegister];
                
            if ( (uint8_t) machine->A < (uint8_t) machine->mem[memoryAddressRegister] )
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;
            
            if ( (uint8_t) (machine->A & 0xF) < (uint8_t) (machine->mem[memoryAddressRegister] & 0xF))
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            PARITY(tmp1)
            SIGN(tmp1)
            ZERO(tmp1)
            
            break;
        }
        case 0xBF: {
            // CMP A

            machine->zeroFlag = 1; 
            machine->carryFlag = 0;
            machine->signFlag = 0;
            machine->parityFlag = 1;
            machine->auxCarryFlag = 0;
            
            break;
        }
        case 0xC0: {                            // SECT. 3:
            // RNZ
            
            if(machine->zeroFlag == 0) {
                RETURN
            }
            
            break;
        }
        case 0xC1: {
            // POP BC
            machine->C = machine->mem[ machine->stackPointer ];
            machine->B = machine->mem[ machine->stackPointer + 1 ];
            
            machine->stackPointer += 2;
            break;
        }
        case 0xC2: {
            // JNZ
            
            if(machine->zeroFlag == 0)
                machine->programCounter = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
            else
                machine->programCounter += 3;
            
            instructionLength = 0;
            break;
        }
        case 0xC3: {
            // JMP
            
            machine->programCounter = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
            
            #if SUPPORT_CPM_CALLS
            
            if (machine->programCounter == 0x0000)        // in CP/M saltare a 0x0000 equivale a
                return I8080_HALT;            // riavviare il S.O. (warm boot)
            
            #endif
            
            instructionLength = 0;
            break;
        }
        case 0xC4: {
            // CNZ
            
            if(machine->zeroFlag == 0){
                tmp1 = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
                CALL_DIRECT(tmp1)
            }
            else
                instructionLength = 3;
            
            break;
        }
        case 0xC5: {
            // PUSH BC
            
            machine->mem[ machine->stackPointer - 1 ] = machine->B;
            machine->mem[ machine->stackPointer - 2 ] = machine->C;
            
            machine->stackPointer -= 2;
            break;
        }
        case 0xC6: {
            // ADI - add immediate to A
            tmp1 = machine->A + machine->mem[currentProgramCounter+1];
            
            if ( (tmp1 >> 8) != 0 )
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;
            if ( ( ( (machine->A & 0xF) + (machine->mem[currentProgramCounter+1] & 0xF) ) & 0xF0 ) != 0)
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            ZERO(machine->A)
            SIGN(machine->A)
            PARITY(machine->A)
            
            instructionLength = 2;
            break;
        }
        case 0xC7: {
            // RST 0
            
            CALL_DIRECT(0x0000)

            break;
        }
        case 0xC8: {
            // RZ
            
            if(machine->zeroFlag == 1) {
                 RETURN
            }
            
            break;
        }
        case 0xC9: {
            // RET
             RETURN
            
            break;
        }
        case 0xCA: {
            // JZ
            
            if(machine->zeroFlag == 1)
                machine->programCounter = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
            else
                machine->programCounter += 3;
                
            instructionLength = 0;
            break;
        }
        case 0xCB: {
            // JMP
            
            machine->programCounter = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
            
            instructionLength = 0;
            break;
        }
        case 0xCC: {
            // CZ
            
            if(machine->zeroFlag == 1) {
                tmp1 = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
                CALL_DIRECT(tmp1)
            }
            else
                instructionLength = 3;

            break;
        }
        case 0xCD: {
            // CALL
            
            tmp1 = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
            
            #ifdef SUPPORT_CPM_CALLS
            
            /*
                System (BDOS) calls in CP/M :
                
                LD DE,parameter
                LD C,function
                CALL 0x05
                
                the only functions currently implemented are
                    
                the PRINT STRING routine :
                
                    parameter = address of string
                    function = 9
                
                    the string begins at (address + 3 bytes)
                    and ends with a '$' character.
                
                and the PRINT CHAR routine :
                    
                    parameter = ASCII character (E register)
                    function = 2
            */
            unsigned char* str;
            
            if( tmp1 == 0x0005 ) {
                
                switch (machine->C) {
                    case 9: {
                        printf("\nCP/M PRINT >> ");
                        tmp2 = (machine->D<<8) | machine->E;
                        str = &(machine->mem[tmp2+3]);
                        
                        while (*str != '$') {
                            printf("%c", *str);
                            str++;
                    }
                        
                        break;
                    }
                    case 2: {
                        printf("%c", machine->E);
                        
                        break;
                    }
                    default: {
                        printf("CALL TO UNIMPLEMENTED BDOS ROUTINE: 0x%02X\n", machine->C);
                        break;
                    }
                }
                
                machine->programCounter += 3;
                instructionLength = 0;
            }
            else if (tmp1 == 0x0000)    // haltSignal to CP/M warm boot
                return I8080_HALT;
            else
            #endif
            
            do{
                CALL_DIRECT(tmp1)        // TODO : controllare che funzioni anche quando SUPPORT_CPM_CALLS == 0
            } while(0);
            
            break;
        }
        case 0xCE: {
            // ACI (ADI w/ carry)
            tmp1 = machine->A + machine->mem[currentProgramCounter+1] + machine->carryFlag;
            
            if ( (tmp1 >> 8) != 0 )
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;
            if ( ( ( (machine->A & 0x0F) + (machine->mem[currentProgramCounter+1] & 0x0F) ) & 0xF0 ) != 0)
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            ZERO(machine->A)
            SIGN(machine->A)
            PARITY(machine->A)
            
            instructionLength = 2;
            break;
        }
        case 0xCF: {
            // RST 1
            
            CALL_DIRECT(0x0008)
            
            break;
        }
        case 0xD0: {
            // RNC
            
            if(machine->carryFlag == 0) {
                 RETURN
            }
            
            break;
        }
        case 0xD1: {
            // POP DE
            machine->E = machine->mem[ machine->stackPointer ];
            machine->D = machine->mem[ machine->stackPointer + 1 ];
            
            machine->stackPointer += 2;
            
            break;
        }
        case 0xD2: {
            // JNC
            
            if(machine->carryFlag == 0)
                machine->programCounter = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
            else
                machine->programCounter += 3;
            
            instructionLength= 0;
            break;
        }
        case 0xD3: {
            #if 0
            // TODO !!!!! (anche IN)
            strcpy(data.mnemonic, "OUT");
            
            data.inputValues[0] = + (int) fgetc(ifp);
            
            data.num_inputValues= 1;
            #endif
            
            instructionLength = 2;
            break;
        }
        case 0xD4: {
            // CNC
            
            if(machine->carryFlag == 0) {
                tmp1 = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
                CALL_DIRECT(tmp1)
            }
            else
                instructionLength = 3;
                
            break;
        }
        case 0xD5: {
            // PUSH DE
            machine->mem[ machine->stackPointer - 2 ] = machine->E;
            machine->mem[ machine->stackPointer - 1 ] = machine->D;
            
            machine->stackPointer -= 2;
            
            break;
        }
        case 0xD6: {
            // SUI
            tmp1 = machine->A - machine->mem[currentProgramCounter+1];
            
            if ( (tmp1 >> 8) == 0xFF )
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;
            if ( ( ( (machine->A & 0x0F) - (machine->mem[currentProgramCounter+1] & 0x0F) - machine->carryFlag ) & 0xF0 ) != 0 )
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            ZERO(machine->A)
            SIGN(machine->A)
            PARITY(machine->A)
            
            instructionLength = 2;
            break;
        }
        case 0xD7: {
            // RST 2
            
            CALL_DIRECT(0x0010)
            
            break;
        }
        case 0xD8: {
            // RC
            if(machine->carryFlag == 1){
                 RETURN
            }
            
            break;
        }
        case 0xD9: {
            // RET
             RETURN
            
            break;
        }
        case 0xDA: {
            // JC
            
            if(machine->carryFlag == 1)
                machine->programCounter = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
            else
                machine->programCounter += 3;
            
            instructionLength= 0;
            break;
        }
        case 0xDB: {
            // IN
            // TODO
            
            instructionLength = 2;
            break;
        }
        case 0xDC: {
            // CC
            if(machine->carryFlag == 1) {
                tmp1 = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
                CALL_DIRECT(tmp1)
            }
            else
                instructionLength = 3;
            
            break;
        }
        case 0xDD: {
            // CALL
            
            tmp1 = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
            CALL_DIRECT(tmp1)

            break;
        }
        case 0xDE: {
            // SBI - Subtract Immediate with Borrow
            tmp1 = machine->A - machine->mem[currentProgramCounter+1] - machine->carryFlag;
            
            if ( (tmp1 >> 8) == 0xFF )
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;
            
            if ( ( ( (machine->A & 0x0F) - (machine->mem[currentProgramCounter+1] & 0x0F) - machine->carryFlag ) & 0xF0 ) != 0)
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            machine->A = tmp1;
            
            ZERO(machine->A)
            SIGN(machine->A)
            PARITY(machine->A)
            
            instructionLength = 2;
            break;
        }
        case 0xDF: {
            // RST 3
            
            CALL_DIRECT(0x0018)
            
            break;
        }
        case 0xE0: {
            // RPO
            
            if(machine->parityFlag == 0){
                 RETURN
            }
            
            break;
        }
        case 0xE1: {
            // POP HL
            machine->L = machine->mem[ machine->stackPointer ];
            machine->H = machine->mem[ machine->stackPointer + 1 ];
            
            machine->stackPointer += 2;
            break;
        }
        case 0xE2: {
            // JPO
            
            if(machine->parityFlag == 0)
                machine->programCounter = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
            else
                machine->programCounter += 3;
            
            instructionLength= 0;
            break;
        }
        case 0xE3: {
            // XTHL: swap HL with word [SP]
            //         i.e. [SP] <-> L; [SP+1] <-> H;
            
            tmp1 = (machine->H << 8) + machine->L;
            machine->L = machine->mem[machine->stackPointer];
            machine->H = machine->mem[machine->stackPointer + 1];
            machine->mem[machine->stackPointer] = tmp1 & 255;
            machine->mem[machine->stackPointer + 1] = tmp1 >> 8;
            
            break;
        }
        case 0xE4: {
            // CPO
            
            if(machine->parityFlag == 0) {
                tmp1 = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
                CALL_DIRECT(tmp1)
            }
            else
                instructionLength = 3;
                
            break;
        }
        case 0xE5: {
            // PUSH HL
            machine->mem[ machine->stackPointer - 2] = machine->L;
            machine->mem[ machine->stackPointer - 1] = machine->H;
            
            machine->stackPointer -= 2;
            break;
        }
        case 0xE6: {
            // ANI - logical AND immediate with A
            machine->A = machine->A & machine->mem[currentProgramCounter+1];
            machine->carryFlag = 0;
            machine->auxCarryFlag = 0;
            
            SIGN(machine->A)
            ZERO(machine->A)
            PARITY(machine->A)
            
            instructionLength = 2;
            break;
        }
        case 0xE7: {
            // RST 4
            
            CALL_DIRECT(0x0020)

            break;
        }
        case 0xE8: {
            // RPE
            
            if(machine->parityFlag == 1){
                 RETURN
            }
            
            break;
        }
        case 0xE9: {
            // PCHL: jump to [HL]
            
            machine->programCounter = (machine->H << 8) + machine->L;
            instructionLength = 0;
            
            break;
        }
        case 0xEA: {
            // JPE
            
            if(machine->parityFlag == 1)
                machine->programCounter = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
            else
                machine->programCounter += 3;
            
            instructionLength = 0;
            break;
        }
        case 0xEB: {
            // XCHG - exchange DE and HL
            
            tmp1 = (machine->D << 8) + machine->E;
            
            machine->D = machine->H;
            machine->E = machine->L;
            
            machine->H = tmp1 >> 8;
            machine->L = tmp1 & 255;
            
            break;
        }
        case 0xEC: {
            // CPE
            
            if(machine->parityFlag == 1) {
                tmp1 = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
                CALL_DIRECT(tmp1)
            }
            else
                instructionLength = 3;
                
            break;
        }
        case 0xED: {
            // CALL
        
            tmp1 = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
            CALL_DIRECT(tmp1)
    
            break;
        }
        case 0xEE: {
            // XRI - logical XOR immediate with A
            machine->A = machine->A ^ machine->mem[currentProgramCounter+1];
            
            instructionLength = 2;
            break;
        }
        case 0xEF: {
            // RST 5
            
            CALL_DIRECT(0x0028)

            break;
        }
        case 0xF0: {
            // RP
            
            if(machine->signFlag == 0) {
                 RETURN
            }
            
            break;
        }
        case 0xF1: {
            // POP PSW = F:A
            
            machine->A = machine->mem[ machine->stackPointer ];
            tmp1 = machine->mem[ machine->stackPointer + 1 ];
            
            // restore flags:
            machine->signFlag = (tmp1 >> 7) & 1;
            machine->zeroFlag = (tmp1 >> 6) & 1;
            machine->auxCarryFlag = (tmp1 >> 4) & 1;
            machine->parityFlag = (tmp1 >> 2) & 1;
            machine->carryFlag = tmp1 & 1;
            
            machine->stackPointer += 2;
            break;
        }
        case 0xF2: {
            // JP
            if(machine->signFlag == 0)
                machine->programCounter = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
            else
                machine->programCounter += 3;
            
            instructionLength = 0;
            break;
        }
        case 0xF3: {
            // DI: disable interrupts
            machine->interrupts= 0;
            
            break;
        }
        case 0xF4: {
            // CP
            
            if(machine->signFlag == 0) {
                tmp1 = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
                CALL_DIRECT(tmp1)
            }
            else
                instructionLength = 3;
                
            break;
        }
        case 0xF5: {
            // PUSH PSW
            
            machine->mem[ machine->stackPointer - 2 ] = machine->A;
            tmp1 = 0;
            
                // store flags:
            tmp1 += (machine->signFlag) * (1 << 7);    // sign
            tmp1 += (machine->zeroFlag) * (1 << 6);    // zero
            tmp1 += (machine->auxCarryFlag) * (1 << 4);    // aux. carry
            tmp1 += (machine->parityFlag) * (1 << 2);    // parity
            tmp1 += 2;                    // 1 flag (machine->i)
            tmp1 += machine->carryFlag;            // carry
                //
                // remember: SZ0A0P1C
            
            machine->mem[ machine->stackPointer - 1 ] = tmp1;
            machine->stackPointer -= 2;
            
            break;
        }
        case 0xF6: {
            // ORI - logical OR immediate with A
            machine->A = machine->A | machine->mem[currentProgramCounter+1];
            
            machine->carryFlag = 0;
            machine->auxCarryFlag = 0;
            
            SIGN(machine->A)
            ZERO(machine->A)
            PARITY(machine->A)
            
            instructionLength = 2;
            break;
        }
        case 0xF7: {
            // RST 6
            
            CALL_DIRECT(0x0030)
            
            break;
        }
        case 0xF8: {
            // RM
            
            if(machine->signFlag == 1) {
                 RETURN
            }
            
            break;
        }
        case 0xF9: {
            // SPHL: set SP to HL
            
            machine->stackPointer = (machine->H << 8) + machine->L;
            
            break;
        }
        case 0xFA: {
            // JM
            
            if(machine->signFlag == 1)
                machine->programCounter = (machine->mem[currentProgramCounter+2] << 8 ) + machine->mem[currentProgramCounter+1];
            else
                machine->programCounter += 3;
            
            instructionLength = 0;
            break;
        }
        case 0xFB: {
            // EI: enable interrupts
            machine->interrupts= 1;
            
            break;
        }
        case 0xFC: {
            // CM
            
            if(machine->signFlag == 1) {
                tmp1 = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
                CALL_DIRECT(tmp1)
            }
            else
                instructionLength = 3;
                
            break;
        }
        case 0xFD: {
            // CALL
            
            tmp1 = (machine->mem[currentProgramCounter+2] << 8) + machine->mem[currentProgramCounter+1];
            CALL_DIRECT(tmp1)

            break;
        }
        case 0xFE: {
            // CPI - compare immediate with A
            
            tmp2 = machine->mem[currentProgramCounter+1];
            tmp1 = machine->A - tmp2;
                
            if ( (uint8_t) machine->A < (uint8_t) tmp2 )
                machine->carryFlag = 1;
            else
                machine->carryFlag = 0;
                
            if ( (uint8_t) (machine->A & 0xF) < (uint8_t) (tmp2 & 0xF))
                machine->auxCarryFlag = 1;
            else
                machine->auxCarryFlag = 0;
            
            PARITY(tmp1)
            SIGN(tmp1)
            ZERO(tmp1)
            
            instructionLength = 2;
            break;
        }
        case 0xFF: {
            // RST 7
            
            CALL_DIRECT(0x0038)

            break;
        }
    }
    
    machine->programCounter += instructionLength;
    
    return (haltSignal) ? (I8080_HALT) : (instructionLength);
}