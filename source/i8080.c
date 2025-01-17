#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "../include/i8080.h"

/* ----------- BYTE PARITY LOOKUP TABLE ----------- */

#define P2(n) n, n ^ 1, n ^ 1, n
#define P4(n) P2(n), P2(n ^ 1), P2(n ^ 1), P2(n)
#define P6(n) P4(n), P4(n ^ 1), P4(n ^ 1), P4(n)
#define LOOK_UP P6(0), P6(1), P6(1), P6(0)

/* ------------ FLAG UPDATE MACROS ----------------*/

#define SIGN(x) \
            machine->signFlag = ((x) >> 7) ;

#define ZERO(x) \
            machine->zeroFlag = ((x) == 0);

#define PARITY(x) \
            machine->parityFlag = ~ table[(x) & 0xff];

#define CARRY(x) \
            machine->carryFlag = (((x) & 0x0100) != 0);

// sets the aux. carry flag according to the result of the operation x + y
#define AUXCARRY_ADD(x,y) \
            machine->auxCarryFlag = ((((x & 0x0F)+(y & 0x0F)) & 0x10) == 0x10);

#define AUXCARRY_SUB(x,y) \
            AUXCARRY_ADD(x, (~(y)+1))

const uint8_t table[256] = { LOOK_UP };

/* ----------- ACCESS MACROS ----------- */

// read value of a register pair
#define GET_REGISTER_PAIR(x,y) \
            (((x) << 8) + (y))

// write value to a register pair
#define SET_REGISTER_PAIR(x,y,v) \
            x = v >> 8;          \
            y = v & 0x00FF;

// write a 2-byte value to memory at address x
#define WRITE_16BIT_TO_MEM(x, v)            \
            machine->mem[x] = v & 0xFF;     \
            machine->mem[x+1] = ((v) >> 8);

// read a 2-byte value from memory at address x
#define READ_16BIT_FROM_MEM(x) \
            (machine->mem[x+1] << 8) + machine->mem[x]

// read an immediate 2-byte value
#define READ_16BIT_IMMEDIATE \
            READ_16BIT_FROM_MEM(currentProgramCounter+1)

#define PUSH_16BIT(x)                                   \
    machine->stackPointer -= 2;                         \
    WRITE_16BIT_TO_MEM(machine->stackPointer, (x))

#define POP_16BIT(x)                                \
    x = READ_16BIT_FROM_MEM(machine->stackPointer); \
    machine->stackPointer += 2;

/* ----------- INSTRUCTION MACROS ----------- */

// CALL immediate address or register
#define CALL_IMMEDIATE(x)                           \
            PUSH_16BIT(machine->programCounter+3);  \
            machine->programCounter = (x);          \
            instructionLength = 0;
                
// RET after termination of a subroutine
#define RETURN                                  \
            POP_16BIT(machine->programCounter)  \
            instructionLength = 0;

// INR - increment register
#define INR(x)                              \
            tmp1 = x & 0x10;                \
            x = (uint8_t) x + 1;            \
            if(( x & 0x10 ) != tmp1)        \
                machine->auxCarryFlag = 1;  \
            else                            \
                machine->auxCarryFlag = 0;  \
            SIGN(x)                         \
            ZERO(x)                         \
            PARITY(x)                       \

// DCR - decrement register
#define DCR(x)                              \
            tmp1 = x & 0x0F;                \
            x = (uint8_t) x - 1;            \
            if(( x & 0x0F ) != tmp1)        \
                machine->auxCarryFlag = 1;  \
            else                            \
                machine->auxCarryFlag = 0;  \
            SIGN(x)                         \
            ZERO(x)                         \
            PARITY(x)                       \

// INX - increment register pair (the macro is not used for INX SP)
#define INX(x,y)                \
            if ( y == 0xFF ) {  \
                x = x + 1;      \
                y = 0;          \
            }                   \
            else                \
                y = y + 1;      \

// DCX - decrement register pair (the macro is not used for DCX SP)
#define DCX(x,y)                \
            if ( y == 0x00 ) {  \
                x = x - 1;      \
                y = 0xFF;       \
            }                   \
            else                \
                y = y - 1;      \

// ADD - add a register to A
#define ADD(x)                              \
            tmp1 = machine->A + (x);        \
            CARRY(tmp1)                     \
            AUXCARRY_ADD(machine->A, (x))   \
            machine->A = tmp1;              \
            PARITY(machine->A)              \
            SIGN(machine->A)                \
            ZERO(machine->A)                \

// ADC - add a register to A with carry
#define ADC(x) \
            ADD(x+machine->carryFlag)

// DAD - add register pair to HL (the macro is not used for DAD SP)
#define DAD(x,y)                                        \
            tmp1 = machine->L + y;                      \
            machine->carryFlag = tmp1 >> 8;             \
            tmp2 = machine->H + x + machine->carryFlag; \
            machine->L = tmp1 & 0xFF;                   \
            machine->H = tmp2 & 0xFF;                   \

// SUB - subtract a register from A (the macro is not used for SUB A)
#define SUB(x)                                      \
            tmp1 = machine->A - (x);                \
            tmp2 = machine->A ^ (-(x) + 1);         \
            CARRY(tmp1)                             \
            machine->auxCarryFlag =                 \
                (tmp1 & 0x0010) != (tmp2 & 0x0010); \
            machine->A = tmp1;                      \
            PARITY(machine->A)                      \
            SIGN(machine->A)                        \
            ZERO(machine->A)                        \

// SBB - subtract with borrow
#define SBB(x) \
            SUB((x)+machine->carryFlag)

// ANA - AND a register with A
#define ANA(x)                                  \
            tmp1 = machine->A & (x);            \
            machine->carryFlag = 0;             \
            machine->auxCarryFlag =             \
                ((machine->A | (x)) & 8) != 0;  \
            machine->A = tmp1;                  \
            ZERO(machine->A)                    \
            PARITY(machine->A)                  \
            SIGN(machine->A)                    \

// XOR - XOR a register with A
#define XOR(x)                             \
            machine->A = machine->A ^ (x); \
            machine->carryFlag = 0;        \
            machine->auxCarryFlag = 0;     \
            PARITY(machine->A)             \
            SIGN(machine->A)               \
            ZERO(machine->A)               \

// ORA - OR a register with A
#define ORA(x)                           \
            machine->A = machine->A | x; \
            machine->carryFlag = 0;      \
            machine->auxCarryFlag = 0;   \
            PARITY(machine->A)           \
            SIGN(machine->A)             \
            ZERO(machine->A)             \

// CMP - compare a register with A (the macro is not used for CMP A)
#define CMP(x)                            \
            tmp1 = machine->A - (x);      \
            machine->carryFlag =          \
                (machine->A < (x));       \
            AUXCARRY_SUB(machine->A, (x)) \
            PARITY(tmp1)                  \
            SIGN(tmp1)                    \
            ZERO(tmp1)                    \


void i8080_init(i8080_t* machine) {
    memset(machine, 0, sizeof(i8080_t));
}

uint16_t i8080_register_get(const i8080_t* machine, const i8080_register_t reg) {
    switch (reg) {
        case I8080_REGISTER_A:
            return machine->A;
        case I8080_REGISTER_B:
            return machine->B;
        case I8080_REGISTER_C:
            return machine->C;
        case I8080_REGISTER_D:
            return machine->D;
        case I8080_REGISTER_E:
            return machine->E;
        case I8080_REGISTER_H:
            return machine->H;
        case I8080_REGISTER_L:
            return machine->L;
        
        case I8080_REGISTER_BC:
            return GET_REGISTER_PAIR(machine->B, machine->C);
        case I8080_REGISTER_DE:
            return GET_REGISTER_PAIR(machine->D, machine->E);
        case I8080_REGISTER_HL:
            return GET_REGISTER_PAIR(machine->H, machine->L);

        case I8080_REGISTER_PROGRAM_COUNTER:
            return machine->programCounter;
        case I8080_REGISTER_STACK_POINTER:
            return machine->stackPointer;
        
        default:
            fprintf(stderr, "ERROR: i8080_register_get: unrecognized register\n");
            exit(EXIT_FAILURE);
    }
}

void i8080_register_set(i8080_t* machine, const i8080_register_t reg, const uint16_t value) {
    switch (reg) {
        case I8080_REGISTER_A:
            machine->A = value & 0x00FF;
            break;
        case I8080_REGISTER_B:
            machine->B = value & 0x00FF;
            break;
        case I8080_REGISTER_C:
            machine->C = value & 0x00FF;
            break;
        case I8080_REGISTER_D:
            machine->D = value & 0x00FF;
            break;
        case I8080_REGISTER_E:
            machine->E = value & 0x00FF;
            break;
        case I8080_REGISTER_H:
            machine->H = value & 0x00FF;
            break;
        case I8080_REGISTER_L:
            machine->L = value & 0x00FF;
            break;
        
        case I8080_REGISTER_BC:
            SET_REGISTER_PAIR(machine->B, machine->C, value);
            break;
        case I8080_REGISTER_DE:
            SET_REGISTER_PAIR(machine->D, machine->E, value);
            break;
        case I8080_REGISTER_HL:
            SET_REGISTER_PAIR(machine->H, machine->L, value);
            break;
        
        case I8080_REGISTER_PROGRAM_COUNTER:
            machine->programCounter = value;
            break;
        case I8080_REGISTER_STACK_POINTER:
            machine->stackPointer = value;
            break;

        default:
            fprintf(stderr, "ERROR: i8080_register_set: unrecognized register\n");
            exit(EXIT_FAILURE);
    }
}

bool i8080_flag_get(const i8080_t* machine, const i8080_flag_t flag) {
    switch(flag) {
        case I8080_FLAG_SIGN:
            return machine->signFlag;
        case I8080_FLAG_ZERO:
            return machine->zeroFlag;
        case I8080_FLAG_CARRY:
            return machine->carryFlag;
        case I8080_FLAG_AUXCARRY:
            return machine->auxCarryFlag;
        case I8080_FLAG_PARITY:
            return machine->parityFlag;
        
        default:
            fprintf(stderr, "ERROR: i8080_flag_get: unrecognized flag\n");
            exit(EXIT_FAILURE);
    }
}

void i8080_flag_set(i8080_t* machine, const i8080_flag_t flag, const bool value) {
    switch(flag) {
        case I8080_FLAG_SIGN:
            machine->signFlag = value;
            break;
        case I8080_FLAG_ZERO:
            machine->zeroFlag = value;
            break;
        case I8080_FLAG_CARRY:
            machine->carryFlag = value;
            break;
        case I8080_FLAG_AUXCARRY:
            machine->auxCarryFlag = value;
            break;
        case I8080_FLAG_PARITY:
            machine->parityFlag = value;
            break;
        
        default:
            fprintf(stderr, "ERROR: i8080_flag_set: unrecognized flag\n");
            exit(EXIT_FAILURE);
    }
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
    const uint16_t memoryAddressRegister = (machine->H << 8) + machine->L;  // memory address "virtual register" [HL]

    uint8_t instructionLength = 1;  // default value
    uint16_t tmp1, tmp2;
    
    switch (instruction) {
        case 0x00:
        case 0x08:
        case 0x10:
        case 0x18:
        case 0x20:
        case 0x28:
        case 0x30:
        case 0x38: {
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
            tmp1 = GET_REGISTER_PAIR(machine->B, machine->C);
            machine->mem[tmp1] = machine->A;
            
            break;
        }
        case 0x03: {
            // INX BC
            INX(machine->B, machine->C)
            
            break;
        }
        case 0x04: {
            // INR B
            INR(machine->B)
            
            break;
        }
        case 0x05: {
            // DCR B
            DCR(machine->B)
            
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
        case 0x09: {
            // DAD BC: add register pair to HL
            DAD(machine->B, machine->C)
            
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
            DCX(machine->B, machine->C)
            
            break;
        }
        case 0x0C: {
            // INR C
            INR(machine->C)
            break;
        }
        case 0x0D: {
            // DCR C
            DCR(machine->C)
            
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
        case 0x11: {
            // LXI DE : load immediate to register pair
            
            machine->D = machine->mem[currentProgramCounter+2];
            machine->E = machine->mem[currentProgramCounter+1];
            
            instructionLength = 3;
            break;
        }
        case 0x12: {
            // STAX DE : store accumulator A through register pair DE
            tmp1 = GET_REGISTER_PAIR(machine->D, machine->E);
            machine->mem[tmp1] = machine->A;
            
            break;
        }
        case 0x13: {
            // INX DE
            INX(machine->D, machine->E)
            
            break;
        }
        case 0x14: {
            // INR D
            INR(machine->D)
            
            break;
        }
        case 0x15: {
            // DCR D
            DCR(machine->D)
            
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
        case 0x19: {
            // DAD DE
            DAD(machine->D, machine->E)
                
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
            DCX(machine->D, machine->E)
            
            break;
        }
        case 0x1C: {
            // INR E
            INR(machine->E)
            
            break;
        }
        case 0x1D: {
            // DCR E
            DCR(machine->E)
            
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
            
            tmp1 = machine->carryFlag;
            machine->carryFlag = machine->A & 1;
            machine->A = machine->A >> 1;
            machine->A += tmp1 << 7;
            
            break;
        }
        case 0x21: {
            // LXI HL : load immediate to register pair HL
            
            tmp1 = READ_16BIT_IMMEDIATE;
            SET_REGISTER_PAIR(machine->H, machine->L, tmp1);
            
            instructionLength = 3;
            break;
        }
        case 0x22: {
            // SHLD: store HL to immediate address
            tmp1 = READ_16BIT_IMMEDIATE;
            machine->mem[tmp1] = machine->L;
            machine->mem[tmp1+1] = machine->H;
            
            instructionLength = 3;
            break;
        }
        case 0x23: {
            // INX HL
            INX(machine->H, machine->L)
            
            
            break;
        }
        case 0x24: {
            // INR H
            INR(machine->H)
            
            break;
        }
        case 0x25: {
            // DCR H
            DCR(machine->H)
            
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
        case 0x29: {
            // DAD HL - add register pair to HL
            DAD(machine->H, machine->L)

            break;
        }
        case 0x2A: {
            // LHLD HL - load HL from immediate address
    
            tmp1 = READ_16BIT_IMMEDIATE;
            machine->L = machine->mem[tmp1];
            machine->H = machine->mem[tmp1+1];
            
            instructionLength = 3;
            break;
        }
        case 0x2B: {
            // DCX HL
            DCX(machine->H, machine->L)
            
            break;
        }
        case 0x2C: {
            // INR L
            INR(machine->L)
            
            break;
        }
        case 0x2D: {
            // DCR L
            DCR(machine->L)
            
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
        case 0x31: {
            // LXI SP
            machine->stackPointer = READ_16BIT_IMMEDIATE;
            
            instructionLength = 3;
            break;
        }
        case 0x32: {
            // STA - store A direct
            
            tmp1 = READ_16BIT_IMMEDIATE;
            machine->mem[tmp1] = machine->A;
            
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
            INR(machine->mem[memoryAddressRegister])
            
            break;
        }
        case 0x35: {
            // DCR M
            DCR(machine->mem[memoryAddressRegister])
            
            break;
        }
        case 0x36: {
            // MVI M
            machine->mem[memoryAddressRegister] = machine->mem[currentProgramCounter+1];
            
            instructionLength = 2;
            break;
        }
        case 0x37: {
            // STC
            machine->carryFlag = 1;
            
            break;
        }
        case 0x39: {
            // DAD SP - add register pair to HL
            
            tmp1 = machine->L + (machine->stackPointer & 0x00FF);
            tmp2 = machine->H + (machine->stackPointer >> 8) + (tmp1 >> 8);
            machine->L = tmp1 & 0xFF;
            machine->H = tmp2 & 0xFF;
            CARRY(tmp2)

            break;
        }
        case 0x3A: {
            // LDA: load A direct
            tmp1 = READ_16BIT_IMMEDIATE;
            machine->A = machine->mem[tmp1];
            
            instructionLength = 3;
            break;
        }
        case 0x3B: {
            // DCX SP
            machine->stackPointer = (uint16_t) machine->stackPointer-1;
            
            break;
        }
        case 0x3C: {
            // INR A
            INR(machine->A)
            
            break;
        }
        case 0x3D: {
            // DCR A
            DCR(machine->A)
            
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
            
            machine->carryFlag = ~ machine->carryFlag;
            
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
            ADD(machine->B);
            
            break;
        }
        case 0x81: {
            // ADD C
            ADD(machine->C)
            
            break;
        }
        case 0x82: {
            // ADD D
            ADD(machine->D)
            
            break;
        }
        case 0x83: {
            // ADD E
            ADD(machine->E)
            
            break;
        }
        case 0x84: {
            // ADD H
            ADD(machine->H)
            
            break;
        }
        case 0x85: {
            // ADD L
            ADD(machine->L)
            
            break;
        }
        case 0x86: {
            // ADD M
            ADD(machine->mem[memoryAddressRegister])
            
            break;
        }
        case 0x87: {
            // ADD A
            ADD(machine->A)
            
            break;
        }
        case 0x88: {
            // ADC B
            ADC(machine->B)
            
            break;
        }
        case 0x89: {
            // ADC C
            ADC(machine->C)
            
            break;
        }
        case 0x8A: {
            // ADC D
            ADC(machine->D)
            
            break;
        }
        case 0x8B: {
            // ADC E
            ADC(machine->E)
            
            break;
        }
        case 0x8C: {
            // ADC H
            ADC(machine->H)
            
            break;
        }
        case 0x8D: {
            // ADC L
            ADC(machine->L)
            
            break;
        }
        case 0x8E: {
            // ADC M
            ADC(machine->mem[memoryAddressRegister])
            
            break;
        }
        case 0x8F: {
            // ADC A
            ADC(machine->A)
            
            break;
        }
        case 0x90: {
            // SUB B
            SUB(machine->B)
            
            break;
        }
        case 0x91: {
            // SUB C
            SUB(machine->C)
            
            break;
        }
        case 0x92: {
            // SUB D
            SUB(machine->D)
            
            break;
        }
        case 0x93: {
            // SUB E
            SUB(machine->E)
            
            break;
        }
        case 0x94: {
            // SUB H
            SUB(machine->H)
            
            break;
        }
        case 0x95: {
            // SUB L
            SUB(machine->L)
            
            break;
        }
        case 0x96: {
            // SUB M
            SUB(machine->mem[memoryAddressRegister])
            
            break;
        }
        case 0x97: {
            // SUB A
            machine->A = 0;
            machine->carryFlag = 0;
            machine->parityFlag = 1;
            machine->zeroFlag = 1;
            machine->signFlag = 0;
            
            break;
        }
        case 0x98: { // SBB - subtract with borrow
            // SBB B
            SBB(machine->B)
            
            break;
        }
        case 0x99: {
            // SBB C
            SBB(machine->C)
            
            break;
        }
        case 0x9A: {
            // SBB D
            SBB(machine->D)
            
            break;
        }
        case 0x9B: {
            // SBB E
            SBB(machine->E)
            
            break;
        }
        case 0x9C: {
            // SBB H
            SBB(machine->H)
            
            break;
        }
        case 0x9D: {
            // SBB L
            SBB(machine->L)
            
            break;
        }
        case 0x9E: {
            // SBB M
            SBB(machine->mem[memoryAddressRegister])
            
            break;
        }
        case 0x9F: {
            // SBB A
            SBB(machine->A)
            
            break;
        }
        case 0xA0: {
            // ANA B
            ANA(machine->B)
            
            break;
        }
        case 0xA1: {
            // ANA C
            ANA(machine->C)
            
            break;
        }
        case 0xA2: {
            // ANA D
            ANA(machine->D)
            
            break;
        }
        case 0xA3: {
            // ANA E
            ANA(machine->E)
            
            break;
        }
        case 0xA4: {
            // ANA H
            ANA(machine->H)
            
            break;
        }
        case 0xA5: {
            // ANA L
            ANA(machine->L)
            
            break;
        }
        case 0xA6: {
            // ANA M
            ANA(machine->mem[memoryAddressRegister])
            
            break;
        }
        case 0xA7: {
            // ANA A
            ANA(machine->A)
            
            break;
        }
        case 0xA8: {
            // XOR B
            XOR(machine->B)
            
            break;
        }
        case 0xA9: {
            // XOR C
            XOR(machine->C)
            
            break;
        }
        case 0xAA: {
            // XOR D
            XOR(machine->D)
            
            break;
        }
        case 0xAB: {
            // XOR E
            XOR(machine->E)
            
            break;
        }
        case 0xAC: {
            // XOR H
            XOR(machine->H)
            
            break;
        }
        case 0xAD: {
            // XOR L
            XOR(machine->L)
            
            break;
        }
        case 0xAE: {
            // XOR M
            XOR(machine->mem[memoryAddressRegister])
            
            break;
        }
        case 0xAF: {
            // XOR A
            XOR(machine->A)
            
            break;
        }
        case 0xB0: {
            // ORA B
            ORA(machine->B)
            
            break;
        }
        case 0xB1: {
            // ORA C
            ORA(machine->C)
            
            break;
        }
        case 0xB2: {
            // ORA D
            ORA(machine->D)
            
            break;
        }
        case 0xB3: {
            // ORA E
            ORA(machine->E)
            
            break;
        }
        case 0xB4: {
            // ORA H
            ORA(machine->H)
            
            break;
        }
        case 0xB5: {
            // ORA L
            ORA(machine->L)
            
            break;
        }
        case 0xB6: {
            // ORA M
            ORA(machine->mem[memoryAddressRegister])
            
            break;
        }
        case 0xB7: {
            // ORA A
            ORA(machine->A)
            
            break;
        }
        case 0xB8: {
            // CMP B
            CMP(machine->B)
            
            break;
        }
        case 0xB9: {
            // CMP C
            CMP(machine->C)
            
            break;
        }
        case 0xBA: {
            // CMP D
            CMP(machine->D)
            
            break;
        }
        case 0xBB: {
            // CMP E
            CMP(machine->E)
            
            break;
        }
        case 0xBC: {
            // CMP H
            CMP(machine->H)
            
            break;
        }
        case 0xBD: {
            // CMP L
            CMP(machine->L)
            
            break;
        }
        case 0xBE: {
            // CMP M
            CMP(machine->mem[memoryAddressRegister])
            
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
        case 0xC0: {
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
                machine->programCounter = READ_16BIT_IMMEDIATE;
            else
                machine->programCounter += 3;
            
            instructionLength = 0;
            break;
        }
        case 0xC3: {
            // JMP
            
            machine->programCounter = READ_16BIT_IMMEDIATE;
            
            instructionLength = 0;
            break;
        }
        case 0xC4: {
            // CNZ
            
            if(machine->zeroFlag == 0){
                tmp1 = READ_16BIT_IMMEDIATE;
                CALL_IMMEDIATE(tmp1)
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

            ADD(machine->mem[currentProgramCounter+1])
            
            instructionLength = 2;
            break;
        }
        case 0xC7: {
            // RST 0
            
            CALL_IMMEDIATE(0x0000)

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
                machine->programCounter = READ_16BIT_IMMEDIATE;
            else
                machine->programCounter += 3;
                
            instructionLength = 0;
            break;
        }
        case 0xCB: {
            // JMP
            
            machine->programCounter = READ_16BIT_IMMEDIATE;
            
            instructionLength = 0;
            break;
        }
        case 0xCC: {
            // CZ
            
            if(machine->zeroFlag == 1) {
                tmp1 = READ_16BIT_IMMEDIATE;
                CALL_IMMEDIATE(tmp1)
            }
            else
                instructionLength = 3;

            break;
        }
        case 0xCD: {
            // CALL
            
            tmp1 = READ_16BIT_IMMEDIATE;
            CALL_IMMEDIATE(tmp1)
            
            break;
        }
        case 0xCE: {
            // ACI (ADI w/ carry)

            ADC(machine->mem[currentProgramCounter+1]);
            
            instructionLength = 2;
            break;
        }
        case 0xCF: {
            // RST 1
            
            CALL_IMMEDIATE(0x0008)
            
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
                machine->programCounter = READ_16BIT_IMMEDIATE;
            else
                machine->programCounter += 3;
            
            instructionLength= 0;
            break;
        }
        case 0xD3: {
            // OUT
            // TODO
            
            instructionLength = 2;
            break;
        }
        case 0xD4: {
            // CNC
            
            if(machine->carryFlag == 0) {
                tmp1 = READ_16BIT_IMMEDIATE;
                CALL_IMMEDIATE(tmp1)
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

            SUB(machine->mem[currentProgramCounter+1]);
            
            instructionLength = 2;
            break;
        }
        case 0xD7: {
            // RST 2
            
            CALL_IMMEDIATE(0x0010)
            
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
                machine->programCounter = READ_16BIT_IMMEDIATE;
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
                tmp1 = READ_16BIT_IMMEDIATE;
                CALL_IMMEDIATE(tmp1)
            }
            else
                instructionLength = 3;
            
            break;
        }
        case 0xDD: {
            // CALL
            
            tmp1 = READ_16BIT_IMMEDIATE;
            CALL_IMMEDIATE(tmp1)

            break;
        }
        case 0xDE: {
            // SBI - Subtract Immediate with Borrow

            SBB(machine->mem[currentProgramCounter+1])
            
            instructionLength = 2;
            break;
        }
        case 0xDF: {
            // RST 3
            
            CALL_IMMEDIATE(0x0018)
            
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
                machine->programCounter = READ_16BIT_IMMEDIATE;
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
            WRITE_16BIT_TO_MEM(machine->stackPointer, tmp1);
            
            break;
        }
        case 0xE4: {
            // CPO
            
            if(machine->parityFlag == 0) {
                tmp1 = READ_16BIT_IMMEDIATE;
                CALL_IMMEDIATE(tmp1)
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
            
            ANA(machine->mem[currentProgramCounter+1])
            
            instructionLength = 2;
            break;
        }
        case 0xE7: {
            // RST 4
            
            CALL_IMMEDIATE(0x0020)

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
                machine->programCounter = READ_16BIT_IMMEDIATE;
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
            machine->L = tmp1 & 0xFF;
            
            break;
        }
        case 0xEC: {
            // CPE

            if(machine->parityFlag == 1) {
                tmp1 = READ_16BIT_IMMEDIATE;
                CALL_IMMEDIATE(tmp1)
            }
            else
                instructionLength = 3;
                
            break;
        }
        case 0xED: {
            // CALL
        
            tmp1 = READ_16BIT_IMMEDIATE;
            CALL_IMMEDIATE(tmp1)
    
            break;
        }
        case 0xEE: {
            // XRI - logical XOR immediate with A

            XOR(machine->mem[currentProgramCounter+1]);
            
            instructionLength = 2;
            break;
        }
        case 0xEF: {
            // RST 5
            
            CALL_IMMEDIATE(0x0028)

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
            
            machine->A = machine->mem[machine->stackPointer+1];
            tmp1 = machine->mem[machine->stackPointer];
            
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
                machine->programCounter = READ_16BIT_IMMEDIATE;
            else
                machine->programCounter += 3;
            
            instructionLength = 0;
            break;
        }
        case 0xF3: {
            // DI: disable interrupts

            machine->interrupts = 0;
            
            break;
        }
        case 0xF4: {
            // CP
            
            if(machine->signFlag == 0) {
                tmp1 = READ_16BIT_IMMEDIATE;
                CALL_IMMEDIATE(tmp1)
            }
            else
                instructionLength = 3;
                
            break;
        }
        case 0xF5: {
            // PUSH PSW
            
            machine->mem[ machine->stackPointer - 1 ] = machine->A;
            tmp1 = 0;
            
            // store flags:
            tmp1 += (1 << 7) * (machine->signFlag);
            tmp1 += (1 << 6) * (machine->zeroFlag);
              // 0 bit
            tmp1 += (1 << 4) * (machine->auxCarryFlag);
              // 0 bit
            tmp1 += (1 << 2) * (machine->parityFlag);
            tmp1 += (1 << 1);
            tmp1 += machine->carryFlag;
            
            machine->mem[machine->stackPointer - 2] = tmp1;
            machine->stackPointer -= 2;
            
            break;
        }
        case 0xF6: {
            // ORI - logical OR immediate with A

            ORA(machine->mem[currentProgramCounter+1])
            
            instructionLength = 2;
            break;
        }
        case 0xF7: {
            // RST 6
            
            CALL_IMMEDIATE(0x0030)
            
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
                machine->programCounter = READ_16BIT_IMMEDIATE;
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
                tmp1 = READ_16BIT_IMMEDIATE;
                CALL_IMMEDIATE(tmp1)
            }
            else
                instructionLength = 3;
                
            break;
        }
        case 0xFD: {
            // CALL
            
            tmp1 = READ_16BIT_IMMEDIATE;
            CALL_IMMEDIATE(tmp1)

            break;
        }
        case 0xFE: {
            // CPI - compare immediate with A
            
            CMP(machine->mem[currentProgramCounter+1]);
            
            instructionLength = 2;
            break;
        }
        case 0xFF: {
            // RST 7
            
            CALL_IMMEDIATE(0x0038)

            break;
        }
    }
    
    machine->programCounter += instructionLength;
    
    return (haltSignal) ? (I8080_HALT) : (instructionLength);
}
