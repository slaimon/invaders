#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "i8080.h"

/* ------------- BYTE PARITY LOOKUP TABLE ----------- */

#define P2(n) n, n ^ 1, n ^ 1, n
#define P4(n) P2(n), P2(n ^ 1), P2(n ^ 1), P2(n)
#define P6(n) P4(n), P4(n ^ 1), P4(n ^ 1), P4(n)
#define LOOK_UP P6(0), P6(1), P6(1), P6(0)

/* ------------- MACRO UTILS: ----------------------- */

// CALL direct address or register
#define CALL_DIRECT(x) \
            state->programCounter += 3 ; \
            state->mem[state->stackPointer-1] = state->programCounter >> 8 ; \
            state->mem[state->stackPointer-2] = state->programCounter & 0xFF ; \
            state->stackPointer -= 2 ; \
            state->programCounter = (x) ; \
            instructionLength = 0 ;
                
// RET after termination of a subroutine
#define RETURN \
            memoryAddressRegister = state->stackPointer ; \
            state->programCounter = (state->mem[memoryAddressRegister+1] << 8) + state->mem[memoryAddressRegister] ; \
            state->stackPointer += 2 ; \
            instructionLength = 0 ;

/* ------------ FLAG UPDATE MACROS ----------------*/

#define SIGN(x) \
            state->signFlag = ((x) >> 7) ;

#define ZERO(x) \
            state->zeroFlag = ( (x) == 0 ) ? (1) : (0) ;

#define PARITY(x) \
            state->parityFlag = ~ table[(x) & 0xff];

uint8_t table[256] = { LOOK_UP };


void i8080_init(i8080_state_t* state) {	
    // set all to zero except stack pointer
    memset(state, 0, sizeof(i8080_state_t));
	state->stackPointer = I8080_MEMSIZE-1 ;
	
	return state ;
}

void i8080_setMemory(i8080_state_t* state, bytestream_t* src, uint16_t offset) {
    if (src->size + offset > I8080_MEMSIZE) {
        fprintf(stderr, "ERROR: i8080_setMemory: not enough memory.\n");
        exit(EXIT_FAILURE);
    }

    memcpy(&state->mem[offset], src->data, src->size);
}

int i8080_execute(i8080_state_t* state ) {
    uint16_t currentProgramCounter;
    uint16_t memoryAddressRegister;
    uint16_t tmp1, tmp2;

    uint8_t instruction;
    uint8_t instructionLength;

    bool haltSignal = false;
    
    if ( state == NULL )
        return I8080_FAIL;
    
    currentProgramCounter = state->programCounter ;
    instruction = state->mem[currentProgramCounter] ;
    memoryAddressRegister = (state->H << 8) + state->L;        // memory address "virtual register" [HL]
    
    instructionLength = 1 ;        // default value
    
    switch (instruction) {
        case 0x00: {
            // NOP
            
            break;
        }
        case 0x01: {
            // LXI BC: load immediate to register pair BC
            
            state->B = state->mem[currentProgramCounter+2];
            state->C = state->mem[currentProgramCounter+1];
            
            instructionLength = 3;
            break;
        }
        case 0x02: {
            // STAX BC: store A through register pair BC
            memoryAddressRegister = (state->B << 8) + state->C;
            state->mem[memoryAddressRegister] = state->A;
            
            break;
        }
        case 0x03: {
            // INX BC

            if ( state->C == 0xFF )    {
                state->B = (uint8_t) state->B + 1 ;
                state->C = 0 ;
            }
            else state->C = (uint8_t) state->C + 1 ;
            
            break;
        }
        case 0x04: {
            // INR B
            
            tmp1 = state->B & 0xF0;
            state->B = 1 + (uint8_t) state->B ;
            
            if( ( state->B & 0xF0 ) != tmp1 )
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            SIGN(state->B)
            ZERO(state->B)
            PARITY(state->B)
            
            break;
        }
        case 0x05: {
            // DCR B
            tmp1 = state->B & 0xF0;
            state->B = (uint8_t) state->B - 1 ;
            
            if( ( state->B & 0xF0 ) != tmp1 )
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            SIGN(state->B)
            ZERO(state->B)
            PARITY(state->B)
            
            break;
        }
        case 0x06: {
            // MVI B
            state->B = state->mem[currentProgramCounter+1];
            
            instructionLength = 2;
            break;
        }
        case 0x07: {
            // RLC A : rotate left
            
            state->carryFlag = state->A >> 7 ;
            state->A = state->A << 1;
            state->A += state->carryFlag;
            
            break;
        }
        case 0x08: {
            // NOP
            
            break;
        }
        case 0x09: {
            // DAD BC: add register pair to HL
            
            tmp1 = state->L + state->C;
            tmp2 = state->H + state->B + (tmp1 >> 8);
            
            state->L = tmp1 & 255;
            state->H = tmp2 & 255;
            
            if((tmp2 >> 8) != 0)
                state->carryFlag = 1;
                else
                state->carryFlag = 0;
            
            break;
        }
        case 0x0A: {
            // LDAX BC : load A through BC
            tmp1 = (state->B << 8) + state->C;
            state->A = state->mem[tmp1];
            
            break;
        }
        case 0x0B: {
            // DCX BC
            
            if ( state->C == 0x00 )    {
                state->B = (uint8_t) state->B - 1 ;
                state->C = 0xFF ;
            }
            else state->C = (uint8_t) state->C - 1 ;
            
            break;
        }
        case 0x0C: {
            // INR C
            tmp1 = state->C & 0xF0;
            state->C = (uint8_t) state->C + 1 ;
            
            if( ( state->C & 0xF0 ) != tmp1 )
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            SIGN(state->C)
            ZERO(state->C)
            PARITY(state->C)
            
            break;
        }
        case 0x0D: {
            // DCR C
            tmp1 = state->C & 0xF0;
            state->C = (uint8_t) state->C - 1 ;
            
            if( ( state->C & 0xF0 ) != tmp1 )
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            SIGN(state->C)
            ZERO(state->C)
            PARITY(state->C)
            
            break;
        }
        case 0x0E: {
            // MVI C
            state->C = state->mem[currentProgramCounter+1] ;
            
            instructionLength = 2;
            break;
        }
        case 0x0F: {
            // RRC A : rotate right
            
            state->carryFlag = state->A & 1;
            state->A = state->A >> 1;
            state->A += state->carryFlag << 7;
            
            instructionLength = 1;
            break;
        }
        case 0x10: {
            // NOP
            
            break;
        }
        case 0x11: {
            // LXI DE : load immediate to register pair
            
            state->D = state->mem[currentProgramCounter+2];
            state->E = state->mem[currentProgramCounter+1];
            
            instructionLength = 3;
            break;
        }
        case 0x12: {
            // STAX DE : store accumulator A through register pair DE
            memoryAddressRegister = (state->D << 8) + state->E;
            state->mem[memoryAddressRegister] = state->A;
            
            break;
        }
        case 0x13: {
            // INX DE
            
            if ( state->E == 0xFF )    {
                state->D = 1 + (uint8_t) state->D ;
                state->E = 0 ;
            }
            else state->E = 1 + (uint8_t) state->E ;
            
            break;
        }
        case 0x14: {
            // INR D
            tmp1 = state->D & 0xF0;
            state->D = (uint8_t) state->D + 1 ;
            
            if( ( state->D & 0xF0 ) != tmp1 )
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            SIGN(state->D)
            ZERO(state->D)
            PARITY(state->D)
            
            break;
        }
        case 0x15: {
            // DCR D
            tmp1 = state->D & 0xF0;
            state->D = (uint8_t) state->D - 1 ;
            
            if( (state->D & 0xF0 ) != tmp1 )
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            SIGN(state->D)
            ZERO(state->D)
            PARITY(state->D)
            
            break;
        }
        case 0x16: {
            // MVI D
            state->D = state->mem[currentProgramCounter+1];
            
            instructionLength = 2;
            break;
        }
        case 0x17: {
            // RAL : rotate A through carry bit
            
            tmp1 = state->carryFlag;
            state->carryFlag = state->A >> 7 ;
            state->A = state->A << 1 ;
            state->A += tmp1 ;
            
            break;
        }
        case 0x18: {
            // NOP
            
            break;
        }
        case 0x19: {
            // DAD DE
            tmp1 = state->L + state->E;
            tmp2 = state->H + state->D + (tmp1 >> 8);
            
            state->L = tmp1 & 255;
            state->H = tmp2 & 255;
            
            if( ( tmp2 >> 8 ) != 0 )
                state->carryFlag = 1;
                else
                state->carryFlag = 0;
                
            break;
        }
        case 0x1A: {
            // LDAX DE : load A through DE
            tmp1 = (state->D << 8) + state->E;
            state->A = state->mem[tmp1];
            
            break;
        }
        case 0x1B: {
            // DCX DE

            if ( state->E == 0x00 )    {
                state->D = (uint8_t) state->D - 1 ;
                state->E = 0xFF ;
            }
            else state->E = (uint8_t) state->E - 1 ;
            
            break;
        }
        case 0x1C: {
            // INR E
            tmp1 = state->E & 0xF0;
            state->E = (uint8_t) state->E + 1 ;
            
            if( ( state->E & 0xF0 ) != tmp1 )
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            SIGN(state->E)
            ZERO(state->E)
            PARITY(state->E)
            
            break;
        }
        case 0x1D: {
            // DCR E
            tmp1 = state->E & 0xF0;
            state->E = (uint8_t) state->E - 1 ;
            
            if( ( state->E & 0xF0 ) != tmp1 )
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            SIGN(state->E)
            ZERO(state->E)
            PARITY(state->E)
            
            break;
        }
        case 0x1E: {
            // MVI E
            
            state->E = state->mem[currentProgramCounter+1];
            
            instructionLength = 2;
            break;
        }
        case 0x1F: {
            // RAR : rotate right through carry
            
            tmp1         = state->carryFlag;
            state->carryFlag = state->A & 1;
            state->A = state->A >> 1;
            state->A += tmp1 << 7;
            
            break;
        }
        case 0x20: {
            // NOP
            
            break;
        }
        case 0x21: {
            // LXI HL : load immediate to register pair HL
            
            state->L = state->mem[currentProgramCounter+1];
            state->H = state->mem[currentProgramCounter+2];
            
            instructionLength = 3;
            break;
        }
        case 0x22: {
            // SHLD: store HL direct
            memoryAddressRegister = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1];
            state->mem[memoryAddressRegister] = state->L;
            state->mem[memoryAddressRegister+1] = state->H;
            
            instructionLength = 3;
            break;
        }
        case 0x23: {
            // INX HL
            
            if ( state->L == 0xFF )    {
                state->H = (uint8_t) state->H + 1 ;
                state->L = 0 ;
            }
            else state->L = (uint8_t) state->L + 1 ;
            
            
            break;
        }
        case 0x24: {
            // INR H
            tmp1 = state->H & 0xF0;
            state->H = (uint8_t) state->H + 1 ;
            
            if( ( state->H & 0xF0 ) != tmp1 )
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            SIGN(state->H)
            ZERO(state->H)
            PARITY(state->H)
            
            break;
        }
        case 0x25: {
            // DCR H
            tmp1 = state->H & 0xF0;
            state->H = (uint8_t) state->H - 1 ;
            
            if( ( state->H & 0xF0 ) != tmp1 )
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            SIGN(state->H)
            ZERO(state->H)
            PARITY(state->H)
            
            break;
        }
        case 0x26: {
            // MVI H
            state->H = state->mem[currentProgramCounter+1];
            
            instructionLength = 2;
            break;
        }
        case 0x27: {
            // DAA - Decimal Adjust Accumulator : SZAPC
            
            tmp1 = state->A & 0xF0;
            
            if( (state->A & 0x0F) > 9 || state->auxCarryFlag == 1)
                state->A += 6;
            if( (state->A & 0xF0) != tmp1)
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
                    
            tmp1 = state->A;
            
            if( ( (state->A & 0xF0) >> 4 ) > 9 || state->carryFlag == 1)
                tmp1 += 0x60;
            if( tmp1 >> 8 != 0 )
                state->carryFlag = 1;
                else
                state->carryFlag = 0;
            
            state->A = tmp1 & 0xFF;
            
            SIGN(state->A)
            ZERO(state->A)
            PARITY(state->A)
            
            break;
        }
        case 0x28: {
            // NOP
            
            break;
        }
        case 0x29: {
            // DAD HL - add register pair to HL
            
            tmp1 = state->L + state->L;
            tmp2 = state->H + state->H + (tmp1 >> 8);
            
            state->L = tmp1 & 255;
            state->H = tmp2 & 255;
            
            if((tmp2 >> 8) != 0)
                state->carryFlag = 1;
                else
                state->carryFlag = 0;

            break;
        }
        case 0x2A: {
            // LHLD HL - load HL direct
    
            memoryAddressRegister = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1];
            state->H = state->mem[memoryAddressRegister+1];
            state->L = state->mem[memoryAddressRegister];
            
            instructionLength = 3;
            break;
        }
        case 0x2B: {
            // DCX HL
            
            if ( state->L == 0x00 )    {
                state->H = (uint8_t) state->H - 1 ;
                state->L = 0xFF ;
            }
            else state->L = (uint8_t) state->L - 1 ;
            
            break;
        }
        case 0x2C: {
            // INR L
            tmp1 = state->L & 0xF0;
            state->L = (uint8_t) state->L + 1 ;
            
            if( (state->L & 0xF0 ) != tmp1 )
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            SIGN(state->L)
            ZERO(state->L)
            PARITY(state->L)
            
            break;
        }
        case 0x2D: {
            // DCR L
            tmp1 = state->L & 0xF0;
            state->L = (uint8_t) state->L - 1 ;
            
            if( ( state->L & 0xF0 ) != tmp1 )
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            SIGN(state->L)
            ZERO(state->L)
            PARITY(state->L)
            
            break;
        }
        case 0x2E: {
            // MVI L
            state->L = state->mem[currentProgramCounter+1];
            
            instructionLength = 2;
            break;
        }
        case 0x2F: {
            // CMA
            state->A = ~ state->A;
            
            break;
        }
        case 0x30: {
            // NOP
            
            break;
        }
        case 0x31: {
            // LXI SP
            state->stackPointer = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
            
            instructionLength = 3;
            break;
        }
        case 0x32: {
            // STA - store A direct
            
            memoryAddressRegister = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
            state->mem[memoryAddressRegister] = state->A ;
            
            instructionLength = 3;
            break;
        }
        case 0x33: {
            // INX SP
            
            state->stackPointer = (uint16_t) state->stackPointer + 1 ;
            
            break;
        }
        case 0x34: {
            // INR M
            tmp1 = state->mem[memoryAddressRegister] & 0xF0;
            state->mem[memoryAddressRegister] = (uint8_t) state->mem[memoryAddressRegister] + 1 ;
            
            if( ( state->mem[memoryAddressRegister] & 0xF0 ) != tmp1 )
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            SIGN(state->mem[memoryAddressRegister])
            ZERO(state->mem[memoryAddressRegister])
            PARITY(state->mem[memoryAddressRegister])
            
            break;
        }
        case 0x35: {
            // DCR M
            tmp1 = state->mem[memoryAddressRegister] & 0xF0;
            state->mem[memoryAddressRegister] = (uint8_t) state->mem[memoryAddressRegister] - 1 ;
            
            if( ( state->mem[memoryAddressRegister] & 0xF0 ) != tmp1 )
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            SIGN(state->mem[memoryAddressRegister])
            ZERO(state->mem[memoryAddressRegister])
            PARITY(state->mem[memoryAddressRegister])
            
            break;
        }
        case 0x36: {
            // MVI M
            state->mem[memoryAddressRegister] = state->mem[currentProgramCounter + 1] ;
            
            instructionLength = 2;
            break;
        }
        case 0x37: {
            // STC
            state->carryFlag = 1;
            
            break;
        }
        case 0x38: {
            // NOP
            
            break;
        }
        case 0x39: {
            // DAD SP - add register pair to HL
            
            tmp1 = state->L + (state->stackPointer & 0x00FF);
            tmp2 = state->H + (state->stackPointer & 0xFF00) + (tmp1 >> 8);
            
            state->L = tmp1 & 255;
            state->H = tmp2 & 255;
            
            if( (tmp2 >> 8) != 0)
                state->carryFlag = 1;
                else
                state->carryFlag = 0;

            break;
        }
        case 0x3A: {
            // LDA: load A direct
            memoryAddressRegister = (state->mem[currentProgramCounter + 2] << 8) + state->mem[currentProgramCounter + 1] ;
            state->A = state->mem[memoryAddressRegister] ;
            
            instructionLength = 3;
            break;
        }
        case 0x3B: {
            // DCX SP
            
            state->stackPointer = (uint16_t) state->stackPointer - 1 ;
            
            break;
        }
        case 0x3C: {
            // INR A
            tmp1 = state->A & 0xF0;
            state->A = (uint8_t) state->A + 1 ;
            
            if( ( state->A & 0xF0 ) != tmp1 )
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            SIGN(state->A)
            ZERO(state->A)
            PARITY(state->A)
            
            break;
        }
        case 0x3D: {
            // DCR A
            tmp1 = state->A & 0xF0;
            state->A = (uint8_t) state->A - 1 ;
            
            if( ( state->A & 0xF0 ) != tmp1 )
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            SIGN(state->A)
            ZERO(state->A)
            PARITY(state->A)
            
            break;
        }
        case 0x3E: {
            // MVI A
            state->A = state->mem[currentProgramCounter + 1] ;
            
            instructionLength = 2;
            break;
        }
        case 0x3F: {
            // CMC (Complement Carry)
            
            state->carryFlag = (state->carryFlag == 0) ? (1) : (0);
            
            break;
        }
        case 0x40: {
            // MOV B,B
            
            break;
        }
        case 0x41: {
            // MOV B,C
            state->B = state->C;
            
            break;
        }
        case 0x42: {
            // MOV B,D
            state->B = state->D;
            
            break;
        }
        case 0x43: {
            // MOV B,E
            state->B = state->E;
            
            break;
        }
        case 0x44: {
            // MOV B,H
            state->B = state->H;
            
            break;
        }
        case 0x45: {
            // MOV B,L
            state->B = state->L;
            
            break;
        }
        case 0x46: {
            // MOV B,M
            state->B = state->mem[memoryAddressRegister];
            
            break;
        }
        case 0x47: {
            // MOV B,A
            state->B = state->A;
            
            break;
        }
        case 0x48: {
            // MOV C,B
            state->C = state->B;
            
            break;
        }
        case 0x49: {
            // MOV C,C
            
            break;
        }
        case 0x4A: {
            // MOV C,D
            state->C = state->D;
            
            break;
        }
        case 0x4B: {
            // MOV C,E
            state->C = state->E;
            
            break;
        }
        case 0x4C: {
            // MOV C,H
            state->C = state->H;
            
            break;
        }
        case 0x4D: {
            // MOV C,L
            state->C = state->L;
            
            break;
        }
        case 0x4E: {
            // MOV C,M
            state->C = state->mem[memoryAddressRegister];
            
            break;
        }
        case 0x4F: {
            // MOV C,A
            state->C = state->A;
            
            break;
        }
        case 0x50: {
            // MOV D,B
            state->D = state->B;
            
            break;
        }
        case 0x51: {
            // MOV D,C
            state->D = state->C;
            
            break;
        }
        case 0x52: {
            // MOV D,D
            
            break;
        }
        case 0x53: {
            // MOV D,E
            state->D = state->E;
            
            break;
        }
        case 0x54: {
            // MOV D,H
            state->D = state->H;
            
            break;
        }
        case 0x55: {
            // MOV D,L
            state->D = state->L;
            
            break;
        }
        case 0x56: {
            // MOV D,M
            state->D = state->mem[memoryAddressRegister];
            
            break;
        }
        case 0x57: {
            // MOV D,A
            state->D = state->A;
            break;
        }
        case 0x58: {
            // MOV E,B
            state->E = state->B;
            
            break;
        }
        case 0x59: {
            // MOV E,C
            state->E = state->C;
            
            break;
        }
        case 0x5A: {
            // MOV E,D
            state->E = state->D;
            
            break;
        }
        case 0x5B: {
            // MOV E,E
            
            break;
        }
        case 0x5C: {
            // MOV E,H
            state->E = state->H;
            
            break;
        }
        case 0x5D: {
            // MOV E,L
            state->E = state->L;
            
            break;
        }
        case 0x5E: {
            // MOV E,M
            state->E = state->mem[memoryAddressRegister];
            
            break;
        }
        case 0x5F: {
            // MOV E,A
            state->E = state->A;
            
            break;
        }
        case 0x60: {
            // MOV H,B
            state->H = state->B;
            
            break;
        }
        case 0x61: {
            // MOV H,C
            state->H = state->C;
            
            break;
        }
        case 0x62: {
            // MOV H,D
            state->H = state->D;
            
            break;
        }
        case 0x63: {
            // MOV H,E
            state->H = state->E;
            
            break;
        }
        case 0x64: {
            // MOV H,H
            
            break;
        }
        case 0x65: {
            // MOV H,L
            state->H = state->L;
            
            break;
        }
        case 0x66: {
            // MOV H,M
            state->H = state->mem[memoryAddressRegister];
            
            break;
        }
        case 0x67: {
            // MOV H,A
            state->H = state->A;
            
            break;
        }
        case 0x68: {
            // MOV L,B
            state->L = state->B;
            
            break;
        }
        case 0x69: {
            // MOV L,C
            state->L = state->C;
            
            break;
        }
        case 0x6A: {
            // MOV L,D
            state->L = state->D;
            
            break;
        }
        case 0x6B: {
            // MOV L,E
            state->L = state->E;
            
            break;
        }
        case 0x6C: {
            // MOV L,H
            state->L = state->H;
            
            break;
        }
        case 0x6D: {
            // MOV L,L
            
            break;
        }
        case 0x6E: {
            // MOV L,M
            state->L = state->mem[memoryAddressRegister];
            
            break;
        }
        case 0x6F: {
            // MOV L,A
            state->L = state->A;
            
            break;
        }
        case 0x70: {
            // MOV M,B
            state->mem[memoryAddressRegister] = state->B;
            
            break;
        }
        case 0x71: {
            // MOV M,C
            state->mem[memoryAddressRegister] = state->C;
            
            break;
        }
        case 0x72: {
            // MOV M,D
            state->mem[memoryAddressRegister] = state->D;
            
            break;
        }
        case 0x73: {
            // MOV M,E
            state->mem[memoryAddressRegister] = state->E;
            
            break;
        }
        case 0x74: {
            // MOV M,H
            state->mem[memoryAddressRegister] = state->H;
            
            break;
        }
        case 0x75: {
            // MOV M,L
            state->mem[memoryAddressRegister] = state->L;
            
            break;
        }
        case 0x76: {
            // HLT
            
            haltSignal = true;
            
            break;
        }
        case 0x77: {
            // MOV M,A
            state->mem[memoryAddressRegister] = state->A;
            
            break;
        }
        case 0x78: {
            // MOV A,B
            state->A = state->B;
            
            break;
        }
        case 0x79: {
            // MOV A,C
            state->A = state->C;
            
            break;
        }
        case 0x7A: {
            // MOV A,D
            state->A = state->D;
            
            break;
        }
        case 0x7B: {
            // MOV A,E
            state->A = state->E;
            
            break;
        }
        case 0x7C: {
            // MOV A,H
            state->A = state->H;
            
            break;
        }
        case 0x7D: {
            // MOV A,L
            state->A = state->L;
            
            break;
        }
        case 0x7E: {
            // MOV A,M
            state->A = state->mem[memoryAddressRegister];
            
            break;
        }
        case 0x7F: {
            // MOV A,A
            
            break;
        }
        case 0x80: {
            // ADD B
            tmp1 = state->A + state->B ;
            tmp2 = state->A ^ state->B ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 1;
                else
                state->carryFlag = 0;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
                
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x81: {
            // ADD C
            tmp1 = state->A + state->C ;
            tmp2 = state->A ^ state->C ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 1;
                else
                state->carryFlag = 0;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x82: {
            // ADD D
            tmp1 = state->A + state->D ;
            tmp2 = state->A ^ state->D ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 1;
                else
                state->carryFlag = 0;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x83: {
            // ADD E
            tmp1 = state->A + state->E ;
            tmp2 = state->A ^ state->E ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 1;
                else
                state->carryFlag = 0;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x84: {
            // ADD H
            tmp1 = state->A + state->H ;
            tmp2 = state->A ^ state->H ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 1;
                else
                state->carryFlag = 0;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x85: {
            // ADD L
            tmp1 = state->A + state->L ;
            tmp2 = state->A ^ state->L ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 1;
                else
                state->carryFlag = 0;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x86: {
            // ADD M
            tmp1 = state->A + state->mem[memoryAddressRegister] ;
            tmp2 = state->A ^ state->mem[memoryAddressRegister] ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 1;
                else
                state->carryFlag = 0;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x87: {
            // ADD A
            tmp1 = state->A + state->A ;
            tmp2 = state->A ^ state->A ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 1;
                else
                state->carryFlag = 0;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x88: {
            //    ADC B
            tmp1 = state->A + state->B + state->carryFlag ;
            tmp2 = state->A ^ (state->B + state->carryFlag) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 1;
                else
                state->carryFlag = 0;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
                
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x89: {
            //    ADC C
            tmp1 = state->A + state->C + state->carryFlag ;
            tmp2 = state->A ^ (state->C + state->carryFlag) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 1;
                else
                state->carryFlag = 0;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x8A: {
            //    ADC D
            tmp1 = state->A + state->D + state->carryFlag ;
            tmp2 = state->A ^ (state->D + state->carryFlag) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 1;
                else
                state->carryFlag = 0;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x8B: {
            // ADC E
            tmp1 = state->A + state->E + state->carryFlag ;
            tmp2 = state->A ^ (state->E + state->carryFlag) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 1;
                else
                state->carryFlag = 0;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x8C: {
            // ADC H
            tmp1 = state->A + state->H + state->carryFlag ;
            tmp2 = state->A ^ (state->H + state->carryFlag) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 1;
                else
                state->carryFlag = 0;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x8D: {
            // ADC L
            tmp1 = state->A + state->L + state->carryFlag ;
            tmp2 = state->A ^ (state->L + state->carryFlag) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 1;
                else
                state->carryFlag = 0;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x8E: {
            // ADC M
            tmp1 = state->A + state->mem[memoryAddressRegister] + state->carryFlag ;
            tmp2 = state->A ^ (state->mem[memoryAddressRegister] + state->carryFlag) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 1;
                else
                state->carryFlag = 0;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x8F: {
            // ADC A
            tmp1 = state->A + state->A + state->carryFlag ;
            tmp2 = state->A ^ (state->A + state->carryFlag) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 1;
                else
                state->carryFlag = 0;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x90: {    // SUB - il carry funziona al contrario!!!
            // SUB B
            tmp1 = state->A - state->B ;
            tmp2 = state->A ^ (- state->B + 1) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 0;
                else
                state->carryFlag = 1;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x91: {
            // SUB C
            tmp1 = state->A - state->C ;
            tmp2 = state->A ^ (- state->C + 1) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 0;
                else
                state->carryFlag = 1;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x92: {
            // SUB D
            tmp1 = state->A - state->D ;
            tmp2 = state->A ^ (- state->D + 1) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 0;
                else
                state->carryFlag = 1;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x93: {
            // SUB E
            tmp1 = state->A - state->E ;
            tmp2 = state->A ^ (- state->E + 1) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 0;
                else
                state->carryFlag = 1;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x94: {
            // SUB H
            tmp1 = state->A - state->H ;
            tmp2 = state->A ^ (- state->H + 1) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 0;
                else
                state->carryFlag = 1;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x95: {
            // SUB L
            tmp1 = state->A - state->L ;
            tmp2 = state->A ^ (- state->L + 1) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 0;
                else
                state->carryFlag = 1;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x96: {
            // SUB M
            tmp1 = state->A - state->mem[memoryAddressRegister] ;
            tmp2 = state->A ^ (- state->mem[memoryAddressRegister] + 1) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 0;
                else
                state->carryFlag = 1;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x97: {
            // SUB A
            tmp1 = state->A - state->A ;
            tmp2 = state->A ^ (- state->A + 1) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 0;
                else
                state->carryFlag = 1;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x98: {            // SBB - subtract with borrow
            // SBB B
            tmp1 = state->A - (state->B + state->carryFlag) ;
            tmp2 = state->A ^ ( -(state->B + state->carryFlag) + 1) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 0;
                else
                state->carryFlag = 1;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x99: {
            // SBB C
            tmp1 = state->A - (state->C + state->carryFlag) ;
            tmp2 = state->A ^ ( -(state->C + state->carryFlag) + 1) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 0;
                else
                state->carryFlag = 1;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x9A: {
            // SBB D
            tmp1 = state->A - (state->D + state->carryFlag) ;
            tmp2 = state->A ^ ( -(state->D + state->carryFlag) + 1) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 0;
                else
                state->carryFlag = 1;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x9B: {
            // SBB E
            tmp1 = state->A - (state->E + state->carryFlag) ;
            tmp2 = state->A ^ ( -(state->E + state->carryFlag) + 1) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 0;
                else
                state->carryFlag = 1;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x9C: {
            // SBB H
            tmp1 = state->A - (state->H + state->carryFlag) ;
            tmp2 = state->A ^ ( -(state->H + state->carryFlag) + 1) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 0;
                else
                state->carryFlag = 1;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x9D: {
            // SBB L
            tmp1 = state->A - (state->L + state->carryFlag) ;
            tmp2 = state->A ^ ( -(state->L + state->carryFlag) + 1) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 0;
                else
                state->carryFlag = 1;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x9E: {
            // SBB M
            tmp1 = state->A - (state->mem[memoryAddressRegister] + state->carryFlag) ;
            tmp2 = state->A ^ ( -(state->mem[memoryAddressRegister] + state->carryFlag) + 1) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 0;
                else
                state->carryFlag = 1;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0x9F: {
            // SBB A
            tmp1 = state->A - (state->A + state->carryFlag) ;
            tmp2 = state->A ^ ( -(state->A + state->carryFlag) + 1) ;
            
            if( (tmp1 & 0xFF00) != 0)
                state->carryFlag = 0;
                else
                state->carryFlag = 1;
            if( (tmp1 & 0x00F0) != (tmp2 & 0x00F0) )
                state->auxCarryFlag = 1;
                else
                state->auxCarryFlag = 0;
            
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xA0: {    // for AND R instructions: AUX.C = ( (A & 8) | (R & 8) )
            // ANA B
            tmp1 = state->A & state->B ;
            
            state->carryFlag = 0;
            state->auxCarryFlag = (state->A & 8) | (state->B & 8) ;
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xA1: {
            // ANA C
            tmp1 = state->A & state->C ;
            
            state->carryFlag = 0 ;
            state->auxCarryFlag = (state->A & 8) | (state->C & 8) ;
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xA2: {
            // ANA D
            tmp1 = state->A & state->D ;
            
            state->carryFlag = 0;
            state->auxCarryFlag = (state->A & 8) | (state->D & 8) ;
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xA3: {
            // ANA E
            tmp1 = state->A & state->E ;
            
            state->carryFlag = 0;
            state->auxCarryFlag = (state->A & 8) | (state->E & 8) ;
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xA4: {
            // ANA H
            tmp1 = state->A & state->H ;
            
            state->carryFlag = 0;
            state->auxCarryFlag = (state->A & 8) | (state->H & 8) ;
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xA5: {
            // ANA L
            tmp1 = state->A & state->L ;
            
            state->carryFlag = 0 ;
            state->auxCarryFlag = (state->A & 8) | (state->L & 8) ;
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xA6: {
            // ANA M
            tmp1 = state->A & state->mem[memoryAddressRegister] ;
            
            state->carryFlag = 0 ;
            state->auxCarryFlag = (state->A & 8) | (state->mem[memoryAddressRegister] & 8) ;
            state->A = tmp1 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xA7: {
            // ANA A
            
            state->carryFlag = 0 ;
            state->auxCarryFlag = state->A & 8 ;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xA8: {
            // XOR B
            state->A = state->A ^ state->B ;
            
            state->carryFlag = 0;
            state->auxCarryFlag = 0;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xA9: {
            // XOR C
            state->A = state->A ^ state->C ;
            
            state->carryFlag = 0;
            state->auxCarryFlag = 0;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xAA: {
            // XOR D
            state->A = state->A ^ state->D ;
            
            state->carryFlag = 0;
            state->auxCarryFlag = 0;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xAB: {
            // XOR E
            state->A = state->A ^ state->E ;
            
            state->carryFlag = 0;
            state->auxCarryFlag = 0;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xAC: {
            // XOR H
            state->A = state->A ^ state->H ;
            
            state->carryFlag = 0;
            state->auxCarryFlag = 0;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xAD: {
            // XOR L
            state->A = state->A ^ state->L ;
            
            state->carryFlag = 0;
            state->auxCarryFlag = 0;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xAE: {
            // XOR M
            state->A = state->A ^ state->mem[memoryAddressRegister] ;
            
            state->carryFlag = 0;
            state->auxCarryFlag = 0;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xAF: {
            // XOR A
            state->A = 0 ;
            
            state->carryFlag = 0;
            state->auxCarryFlag = 0;
            
            PARITY(state->A)
            SIGN(state->A)
            state->zeroFlag = 1 ;
            
            break;
        }
        case 0xB0: {
            // OR B
            state->A = state->A | state->B ;
            
            state->carryFlag = 0;
            state->auxCarryFlag = 0;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xB1: {
            // OR C
            state->A = state->A | state->C ;
            
            state->carryFlag = 0;
            state->auxCarryFlag = 0;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xB2: {
            // OR D
            state->A = state->A | state->D ;
            
            state->carryFlag = 0;
            state->auxCarryFlag = 0;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xB3: {
            // OR E
            state->A = state->A | state->E ;
            
            state->carryFlag = 0;
            state->auxCarryFlag = 0;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xB4: {
            // OR H
            state->A = state->A | state->H ;
            
            state->carryFlag = 0;
            state->auxCarryFlag = 0;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xB5: {
            // OR L
            state->A = state->A | state->L ;
            
            state->carryFlag = 0;
            state->auxCarryFlag = 0;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xB6: {
            // OR M
            state->A = state->A | state->mem[memoryAddressRegister] ;
            
            state->carryFlag = 0;
            state->auxCarryFlag = 0;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xB7: {
            // OR A
            
            state->carryFlag = 0;
            state->auxCarryFlag = 0;
            
            PARITY(state->A)
            SIGN(state->A)
            ZERO(state->A)
            
            break;
        }
        case 0xB8: {
            // CMP B
            tmp1 = state->A - state->B ;
                
            if ( (uint8_t) state->A < (uint8_t) state->B )        // geniale!
                state->carryFlag = 1 ;
                else
                state->carryFlag = 0 ;
                
            if ( (uint8_t) (state->A & 0xF) < (uint8_t) (state->B & 0xF))
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            /*
            if ( (state->A & 128) == ~ (state->B & 128) )
                state->carryFlag = (tmp1 >> 8) & 1 ;
                else
                state->carryFlag = ~ ((tmp1 >> 8) & 1) ;
                
            if ( (state->A & 0x0F) - (state->B & 0x0F) < 0 )
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            */
            PARITY(tmp1)
            SIGN(tmp1)
            ZERO(tmp1)
            
            break;
        }
        case 0xB9: {
            // CMP C
            tmp1 = state->A - state->C ;
                
            if ( (uint8_t) state->A < (uint8_t) state->C )
                state->carryFlag = 1 ;
                else
                state->carryFlag = 0 ;
                
            if ( (uint8_t) (state->A & 0xF) < (uint8_t) (state->C & 0xF))
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            PARITY(tmp1)
            SIGN(tmp1)
            ZERO(tmp1)
            
            break;
        }
        case 0xBA: {
            // CMP D
            tmp1 = state->A - state->D ;
                
            if ( (uint8_t) state->A < (uint8_t) state->D )
                state->carryFlag = 1 ;
                else
                state->carryFlag = 0 ;
            
            if ( (uint8_t) (state->A & 0xF) < (uint8_t) (state->D & 0xF))
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            PARITY(tmp1)
            SIGN(tmp1)
            ZERO(tmp1)
            
            break;
        }
        case 0xBB: {
            // CMP E
            tmp1 = state->A - state->E ;
                
            if ( (uint8_t) state->A < (uint8_t) state->E )
                state->carryFlag = 1 ;
                else
                state->carryFlag = 0 ;
            
            if ( (uint8_t) (state->A & 0xF) < (uint8_t) (state->E & 0xF))
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            PARITY(tmp1)
            SIGN(tmp1)
            ZERO(tmp1)
            
            break;
        }
        case 0xBC: {
            // CMP H
            tmp1 = state->A - state->H ;
                
            if ( (uint8_t) state->A < (uint8_t) state->H )
                state->carryFlag = 1 ;
                else
                state->carryFlag = 0 ;
            
            if ( (uint8_t) (state->A & 0xF) < (uint8_t) (state->H & 0xF))
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            PARITY(tmp1)
            SIGN(tmp1)
            ZERO(tmp1)
            
            break;
        }
        case 0xBD: {
            // CMP L
            tmp1 = state->A - state->L ;
                
            if ( (uint8_t) state->A < (uint8_t) state->L )
                state->carryFlag = 1 ;
                else
                state->carryFlag = 0 ;
            
            if ( (uint8_t) (state->A & 0xF) < (uint8_t) (state->L & 0xF))
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            PARITY(tmp1)
            SIGN(tmp1)
            ZERO(tmp1)
            
            break;
        }
        case 0xBE: {
            // CMP M
            tmp1 = state->A - state->mem[memoryAddressRegister] ;
                
            if ( (uint8_t) state->A < (uint8_t) state->mem[memoryAddressRegister] )
                state->carryFlag = 1 ;
                else
                state->carryFlag = 0 ;
            
            if ( (uint8_t) (state->A & 0xF) < (uint8_t) (state->mem[memoryAddressRegister] & 0xF))
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            PARITY(tmp1)
            SIGN(tmp1)
            ZERO(tmp1)
            
            break;
        }
        case 0xBF: {
            // CMP A

            state->zeroFlag = 1 ; 
            state->carryFlag = 0 ;
            state->signFlag = 0 ;
            state->parityFlag = 1 ;
            state->auxCarryFlag = 0 ;
            
            break;
        }
        case 0xC0: {                            // SECT. 3:
            // RNZ
            
            if(state->zeroFlag == 0) {
                RETURN
            }
            
            break;
        }
        case 0xC1: {
            // POP BC
            state->C = state->mem[ state->stackPointer ] ;
            state->B = state->mem[ state->stackPointer + 1 ] ;
            
            state->stackPointer += 2;
            break;
        }
        case 0xC2: {
            // JNZ
            
            if(state->zeroFlag == 0)
                state->programCounter = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
            else
                state->programCounter += 3 ;
            
            instructionLength = 0;
            break;
        }
        case 0xC3: {
            // JMP
            
            state->programCounter = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
            
            #if SUPPORT_CPM_CALLS
            
            if (state->programCounter == 0x0000)        // in CP/M saltare a 0x0000 equivale a
                return I8080_HALT ;            // riavviare il S.O. (warm boot)
            
            #endif
            
            instructionLength = 0;
            break;
        }
        case 0xC4: {
            // CNZ
            
            if(state->zeroFlag == 0){
                tmp1 = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
                CALL_DIRECT(tmp1)
            }
            else
                instructionLength = 3 ;
            
            break;
        }
        case 0xC5: {
            // PUSH BC
            
            state->mem[ state->stackPointer - 1 ] = state->B ;
            state->mem[ state->stackPointer - 2 ] = state->C ;
            
            state->stackPointer -= 2;
            break;
        }
        case 0xC6: {
            // ADI - add immediate to A
            tmp1 = state->A + state->mem[currentProgramCounter+1] ;
            
            if ( (tmp1 >> 8) != 0 )
                state->carryFlag = 1 ;
                else
                state->carryFlag = 0 ;
            if ( ( ( (state->A & 0xF) + (state->mem[currentProgramCounter+1] & 0xF) ) & 0xF0 ) != 0)
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            state->A = tmp1 ;
            
            ZERO(state->A)
            SIGN(state->A)
            PARITY(state->A)
            
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
            
            if(state->zeroFlag == 1) {
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
            
            if(state->zeroFlag == 1)
                state->programCounter = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
            else
                state->programCounter += 3 ;
                
            instructionLength = 0 ;
            break;
        }
        case 0xCB: {
            // JMP
            
            state->programCounter = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
            
            instructionLength = 0 ;
            break;
        }
        case 0xCC: {
            // CZ
            
            if(state->zeroFlag == 1) {
                tmp1 = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
                CALL_DIRECT(tmp1)
            }
            else
                instructionLength = 3 ;

            break;
        }
        case 0xCD: {
            // CALL
            
            tmp1 = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
            
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
                
                switch (state->C) {
                    case 9: {
                        printf("\nCP/M PRINT >> ");
                        tmp2 = (state->D<<8) | state->E ;
                        str = &(state->mem[tmp2+3]) ;
                        
                        while (*str != '$') {
                            printf("%c", *str) ;
                            str++;
                    }
                        
                        break;
                    }
                    case 2: {
                        printf("%c", state->E);
                        
                        break;
                    }
                    default: {
                        printf("CALL TO UNIMPLEMENTED BDOS ROUTINE: 0x%02X\n", state->C);
                        break;
                    }
                }
                
                state->programCounter += 3 ;
                instructionLength = 0 ;
            }
            else if (tmp1 == 0x0000)    // haltSignal to CP/M warm boot
                return I8080_HALT ;
            else
            #endif
            
            do{
                CALL_DIRECT(tmp1)        // TODO : controllare che funzioni anche quando SUPPORT_CPM_CALLS == 0
            } while(0) ;
            
            break;
        }
        case 0xCE: {
            // ACI (ADI w/ carry)
            tmp1 = state->A + state->mem[currentProgramCounter+1] + state->carryFlag ;
            
            if ( (tmp1 >> 8) != 0 )
                state->carryFlag = 1 ;
                else
                state->carryFlag = 0 ;
            if ( ( ( (state->A & 0x0F) + (state->mem[currentProgramCounter+1] & 0x0F) ) & 0xF0 ) != 0)
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            state->A = tmp1 ;
            
            ZERO(state->A)
            SIGN(state->A)
            PARITY(state->A)
            
            instructionLength = 2 ;
            break;
        }
        case 0xCF: {
            // RST 1
            
            CALL_DIRECT(0x0008)
            
            break;
        }
        case 0xD0: {
            // RNC
            
            if(state->carryFlag == 0) {
                 RETURN
            }
            
            break;
        }
        case 0xD1: {
            // POP DE
            state->E = state->mem[ state->stackPointer ] ;
            state->D = state->mem[ state->stackPointer + 1 ] ;
            
            state->stackPointer += 2;
            
            break;
        }
        case 0xD2: {
            // JNC
            
            if(state->carryFlag == 0)
                state->programCounter = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
            else
                state->programCounter += 3 ;
            
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
            
            instructionLength = 2 ;
            break;
        }
        case 0xD4: {
            // CNC
            
            if(state->carryFlag == 0) {
                tmp1 = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
                CALL_DIRECT(tmp1)
            }
            else
                instructionLength = 3 ;
                
            break;
        }
        case 0xD5: {
            // PUSH DE
            state->mem[ state->stackPointer - 2 ] = state->E ;
            state->mem[ state->stackPointer - 1 ] = state->D ;
            
            state->stackPointer -= 2;
            
            break;
        }
        case 0xD6: {
            // SUI
            tmp1 = state->A - state->mem[currentProgramCounter+1] ;
            
            if ( (tmp1 >> 8) == 0xFF )
                state->carryFlag = 1 ;
                else
                state->carryFlag = 0 ;
            if ( ( ( (state->A & 0x0F) - (state->mem[currentProgramCounter+1] & 0x0F) - state->carryFlag ) & 0xF0 ) != 0 )
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            state->A = tmp1 ;
            
            ZERO(state->A)
            SIGN(state->A)
            PARITY(state->A)
            
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
            if(state->carryFlag == 1){
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
            
            if(state->carryFlag == 1)
                state->programCounter = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
            else
                state->programCounter += 3 ;
            
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
            if(state->carryFlag == 1) {
                tmp1 = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
                CALL_DIRECT(tmp1)
            }
            else
                instructionLength = 3 ;
            
            break;
        }
        case 0xDD: {
            // CALL
            
            tmp1 = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
            CALL_DIRECT(tmp1)

            break;
        }
        case 0xDE: {
            // SBI    (Subtract Immediate w/ Borrow)
            tmp1 = state->A - state->mem[currentProgramCounter+1] - state->carryFlag ;
            
            if ( (tmp1 >> 8) == 0xFF )
                state->carryFlag = 1 ;
                else
                state->carryFlag = 0 ;
            if ( ( ( (state->A & 0x0F) - (state->mem[currentProgramCounter+1] & 0x0F) - state->carryFlag ) & 0xF0 ) != 0)
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
            state->A = tmp1 ;
            
            ZERO(state->A)
            SIGN(state->A)
            PARITY(state->A)
            
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
            
            if(state->parityFlag == 0){
                 RETURN
            }
            
            break;
        }
        case 0xE1: {
            // POP HL
            state->L = state->mem[ state->stackPointer ] ;
            state->H = state->mem[ state->stackPointer + 1 ] ;
            
            state->stackPointer += 2;
            break;
        }
        case 0xE2: {
            // JPO
            
            if(state->parityFlag == 0)
                state->programCounter = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
            else
                state->programCounter += 3 ;
            
            instructionLength= 0;
            break;
        }
        case 0xE3: {
            // XTHL: swap HL with word [SP]
            //         i.e. [SP] <-> L ; [SP+1] <-> H;
            
            tmp1 = (state->H << 8) + state->L;
            state->L = state->mem[state->stackPointer];
            state->H = state->mem[state->stackPointer + 1];
            state->mem[state->stackPointer] = tmp1 & 255;
            state->mem[state->stackPointer + 1] = tmp1 >> 8;
            
            break;
        }
        case 0xE4: {
            // CPO
            
            if(state->parityFlag == 0) {
                tmp1 = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
                CALL_DIRECT(tmp1)
            }
            else
                instructionLength = 3 ;
                
            break;
        }
        case 0xE5: {
            // PUSH HL
            state->mem[ state->stackPointer - 2] = state->L ;
            state->mem[ state->stackPointer - 1] = state->H ;
            
            state->stackPointer -= 2;
            break;
        }
        case 0xE6: {
            // ANI - logical AND immediate with A
            state->A = state->A & state->mem[currentProgramCounter+1] ;
            state->carryFlag = 0;
            state->auxCarryFlag = 0;
            
            SIGN(state->A)
            ZERO(state->A)
            PARITY(state->A)
            
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
            
            if(state->parityFlag == 1){
                 RETURN
            }
            
            break;
        }
        case 0xE9: {
            // PCHL: jump to [HL]
            
            state->programCounter = (state->H << 8) + state->L;
            instructionLength = 0 ;
            
            break;
        }
        case 0xEA: {
            // JPE
            
            if(state->parityFlag == 1)
                state->programCounter = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
            else
                state->programCounter += 3 ;
            
            instructionLength = 0 ;
            break;
        }
        case 0xEB: {
            // XCHG: exchange DE and HL
            
            tmp1 = (state->D << 8) + state->E;
            
            state->D = state->H;
            state->E = state->L;
            
            state->H = tmp1 >> 8;
            state->L = tmp1 & 255;
            
            break;
        }
        case 0xEC: {
            // CPE
            
            if(state->parityFlag == 1) {
                tmp1 = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
                CALL_DIRECT(tmp1)
            }
            else
                instructionLength = 3 ;
                
            break;
        }
        case 0xED: {
            // CALL
        
            tmp1 = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
            CALL_DIRECT(tmp1)
    
            break;
        }
        case 0xEE: {
            // XRI - logical XOR immediate with A
            state->A = state->A ^ state->mem[currentProgramCounter+1] ;
            
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
            
            if(state->signFlag == 0) {
                 RETURN
            }
            
            break;
        }
        case 0xF1: {
            // POP PSW = F:A
            
            state->A = state->mem[ state->stackPointer ] ;
            tmp1 = state->mem[ state->stackPointer + 1 ] ;
            
                // restore flags:
            state->signFlag = (tmp1 >> 7) & 1 ;        // sign
            state->zeroFlag = (tmp1 >> 6) & 1 ;        // zero
            state->auxCarryFlag = (tmp1 >> 4) & 1 ;        // aux. carry
            state->parityFlag = (tmp1 >> 2) & 1 ;        // parity
            state->carryFlag = tmp1 & 1 ;            // carry
                //
                // remember: SZ0A0P1C
            
            state->stackPointer += 2;
            break;
        }
        case 0xF2: {
            // JP
            if(state->signFlag == 0)
                state->programCounter = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
            else
                state->programCounter += 3 ;
            
            instructionLength = 0 ;
            break;
        }
        case 0xF3: {
            // DI: disable interrupts
            state->interrupts= 0;
            
            break;
        }
        case 0xF4: {
            // CP
            
            if(state->signFlag == 0) {
                tmp1 = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
                CALL_DIRECT(tmp1)
            }
            else
                instructionLength = 3 ;
                
            break;
        }
        case 0xF5: {
            // PUSH PSW
            
            state->mem[ state->stackPointer - 2 ] = state->A ;
            tmp1 = 0 ;
            
                // store flags:
            tmp1 += (state->signFlag) * (1 << 7) ;    // sign
            tmp1 += (state->zeroFlag) * (1 << 6) ;    // zero
            tmp1 += (state->auxCarryFlag) * (1 << 4) ;    // aux. carry
            tmp1 += (state->parityFlag) * (1 << 2) ;    // parity
            tmp1 += 2 ;                    // 1 flag (state->i)
            tmp1 += state->carryFlag ;            // carry
                //
                // remember: SZ0A0P1C
            
            state->mem[ state->stackPointer - 1 ] = tmp1 ;
            state->stackPointer -= 2;
            
            break;
        }
        case 0xF6: {
            // ORI - logical OR immediate with A
            state->A = state->A | state->mem[currentProgramCounter+1] ;
            
            state->carryFlag = 0;
            state->auxCarryFlag = 0;
            
            SIGN(state->A)
            ZERO(state->A)
            PARITY(state->A)
            
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
            
            if(state->signFlag == 1) {
                 RETURN
            }
            
            break;
        }
        case 0xF9: {
            // SPHL: set SP to HL
            
            state->stackPointer = (state->H << 8) + state->L;
            
            break;
        }
        case 0xFA: {
            // JM
            
            if(state->signFlag == 1)
                state->programCounter = (state->mem[currentProgramCounter+2] << 8 ) + state->mem[currentProgramCounter+1] ;
            else
                state->programCounter += 3 ;
            
            instructionLength = 0 ;
            break;
        }
        case 0xFB: {
            // EI: enable interrupts
            state->interrupts= 1;
            
            break;
        }
        case 0xFC: {
            // CM
            
            if(state->signFlag == 1) {
                tmp1 = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
                CALL_DIRECT(tmp1)
            }
            else
                instructionLength = 3 ;
                
            break;
        }
        case 0xFD: {
            // CALL
            
            tmp1 = (state->mem[currentProgramCounter+2] << 8) + state->mem[currentProgramCounter+1] ;
            CALL_DIRECT(tmp1)

            break;
        }
        case 0xFE: {
            // CPI - compare immediate with A
            
            tmp2 = state->mem[currentProgramCounter+1] ;
            tmp1 = state->A - tmp2 ;
                
            if ( (uint8_t) state->A < (uint8_t) tmp2 )
                state->carryFlag = 1 ;
                else
                state->carryFlag = 0 ;
                
            if ( (uint8_t) (state->A & 0xF) < (uint8_t) (tmp2 & 0xF))
                state->auxCarryFlag = 1 ;
                else
                state->auxCarryFlag = 0 ;
            
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
    
    state->programCounter += instructionLength ;
    
    return (haltSignal) ? (I8080_HALT) : (instructionLength) ;
}