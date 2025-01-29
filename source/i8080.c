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

const uint8_t parity_table[256] = { LOOK_UP };

/* ------------ FLAG UPDATE MACROS ----------------*/

#define SIGN(x) \
            machine->signFlag = ((x) >> 7) ;

#define ZERO(x) \
            machine->zeroFlag = ((x) == 0);

#define PARITY(x) \
            machine->parityFlag = ~ parity_table[(x) & 0xff];

#define CARRY(x) \
            machine->carryFlag = (((x) & 0x0100) != 0);

// Aux. carry look-up tables. These were computed by Alexander Demin based on a
// real KR580VM80A processor and found at https://github.com/begoon/i8080-core/
// I'm not sure if they can be computed with a more readable logic expression
// but I sure tried
const bool add_auxcarry_table[] = { 0, 0, 1, 0, 1, 0, 1, 1 };
const bool sub_auxcarry_table[] = { 1, 0, 0, 0, 1, 1, 1, 0 };
#define AUXCARRY_INDEX(x,y)     \
            (((x) & 8) >> 1) |  \
            (((y) & 8) >> 2) |  \
            ((tmp1 & 8) >> 3)   \

// sets the aux. carry flag according to the result of the operation x + y
// expects result of operation in tmp1 variable
#define AUXCARRY_ADD(x,y) {                                \
            uint8_t index = AUXCARRY_INDEX(x,y);           \
            machine->auxCarryFlag = add_auxcarry_table[index]; \
}

// sets the aux. carry flag according to the result of the operation x - y
// expects result of operation in tmp1 variable
#define AUXCARRY_SUB(x,y) {                                    \
            uint8_t index = AUXCARRY_INDEX(x,y);               \
            machine->auxCarryFlag = sub_auxcarry_table[index]; \
}

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

#define PUSH_16BIT(x)                                           \
            machine->stackPointer -= 2;                         \
            WRITE_16BIT_TO_MEM(machine->stackPointer, (x))

#define POP_16BIT(x)                                        \
            x = READ_16BIT_FROM_MEM(machine->stackPointer); \
            machine->stackPointer += 2;

/* ----------- INSTRUCTION MACROS ----------- */

// CALL immediate address or register
#define CALL_IMMEDIATE(x)                           \
            PUSH_16BIT(machine->programCounter+3);  \
            machine->programCounter = (x);          \
            instructionLength = 0;                  \
            cycles = 17;

#define CONDITIONAL_CALL(cond)                  \
            if(cond){                           \
                tmp1 = READ_16BIT_IMMEDIATE;    \
                CALL_IMMEDIATE(tmp1)            \
            }                                   \
            else {                              \
                instructionLength = 3;          \
                cycles = 11;                    \
            }

#define CONDITIONAL_JUMP(cond)                                  \
            if(cond)                                            \
                machine->programCounter = READ_16BIT_IMMEDIATE; \
            else                                                \
                machine->programCounter += 3;                   \
            instructionLength = 0;                              \
            cycles = 10;

#define RST(x) \
            PUSH_16BIT(machine->programCounter+1);  \
            machine->programCounter = ((x)<<3);     \
            instructionLength = 0;                  \
            cycles = 11;
                
// RET after termination of a subroutine
#define RETURN                                  \
            POP_16BIT(machine->programCounter)  \
            instructionLength = 0;

#define CONDITIONAL_RETURN(cond) \
            if(cond) {           \
                 RETURN          \
                 cycles = 11;    \
            }                    \
            else                 \
                cycles = 5;

// INR - increment register
#define INR(x)                              \
            x = (uint8_t) x + 1;            \
            machine->auxCarryFlag =         \
                (((x) & 0x0F) == 0x00);     \
            SIGN(x)                         \
            ZERO(x)                         \
            PARITY(x)                       \

// DCR - decrement register
#define DCR(x)                              \
            x = (uint8_t) x - 1;            \
            machine->auxCarryFlag =         \
                (((x) & 0x0F) != 0x0F);     \
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
#define ADC(x)                                              \
            tmp1 = machine->A + (x) + machine->carryFlag;   \
            CARRY(tmp1)                                     \
            AUXCARRY_ADD(machine->A, (x))                   \
            machine->A = tmp1;                              \
            PARITY(machine->A)                              \
            SIGN(machine->A)                                \
            ZERO(machine->A)                                \

// DAD - add register pair to HL (the macro is not used for DAD SP)
#define DAD(x,y)                                    \
            tmp1 = machine->L + y;                  \
            tmp2 = machine->H + x + (tmp1 >> 8);    \
            machine->carryFlag = (tmp2 >> 8);       \
            machine->L = tmp1 & 0xFF;               \
            machine->H = tmp2 & 0xFF;               \

// SUB - subtract a register from A (the macro is not used for SUB A)
#define SUB(x)                                      \
            tmp1 = machine->A - (x);                \
            CARRY(tmp1)                             \
            AUXCARRY_SUB(machine->A, (x))           \
            machine->A = tmp1;                      \
            PARITY(machine->A)                      \
            SIGN(machine->A)                        \
            ZERO(machine->A)                        \

// SBB - subtract with borrow
#define SBB(x)                                              \
            tmp1 = machine->A - (x) - machine->carryFlag;   \
            CARRY(tmp1)                                     \
            AUXCARRY_SUB(machine->A, (x))                   \
            machine->A = tmp1;                              \
            PARITY(machine->A)                              \
            SIGN(machine->A)                                \
            ZERO(machine->A)     

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

int i8080_interrupt(i8080_t* machine, uint8_t restart) {
    if (machine == NULL)
        return I8080_FAIL;
    if (restart > 7)
        return I8080_FAIL;
    
    if (!machine->interrupts)
        return 0;

    PUSH_16BIT(machine->programCounter);
    machine->programCounter = restart << 3;
    return 11;
}

int i8080_execute(i8080_t* machine ) {
    bool haltSignal = false;
    
    if (machine == NULL)
        return I8080_FAIL;
    
    const uint16_t currentProgramCounter = machine->programCounter;
    const uint8_t instruction = machine->mem[currentProgramCounter];
    const uint16_t memoryAddressRegister = (machine->H << 8) + machine->L;  // memory address "virtual register" [HL]

    uint8_t instructionLength = 1;  // default value
    uint16_t tmp1, tmp2;
    
    int cycles;
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
            
            cycles = 4;
            break;
        }
        case 0x01: {
            // LXI BC: load immediate to register pair BC
            
            machine->B = machine->mem[currentProgramCounter+2];
            machine->C = machine->mem[currentProgramCounter+1];
            
            instructionLength = 3;
            cycles = 10;
            break;
        }
        case 0x02: {
            // STAX BC: store A through register pair BC
            tmp1 = GET_REGISTER_PAIR(machine->B, machine->C);
            machine->mem[tmp1] = machine->A;
            
            cycles = 7;
            break;
        }
        case 0x03: {
            // INX BC
            INX(machine->B, machine->C)
            
            cycles = 5;
            break;
        }
        case 0x04: {
            // INR B
            INR(machine->B)
            
            cycles = 5;
            break;
        }
        case 0x05: {
            // DCR B
            DCR(machine->B)
            
            cycles = 5;
            break;
        }
        case 0x06: {
            // MVI B
            machine->B = machine->mem[currentProgramCounter+1];
            
            instructionLength = 2;
            cycles = 7;
            break;
        }
        case 0x07: {
            // RLC A - rotate left
            
            machine->carryFlag = machine->A >> 7;
            machine->A = machine->A << 1;
            machine->A += machine->carryFlag;
            
            cycles = 4;
            break;
        }
        case 0x09: {
            // DAD BC - add register pair to HL
            DAD(machine->B, machine->C)
            
            cycles = 10;
            break;
        }
        case 0x0A: {
            // LDAX BC - load A through BC
            tmp1 = (machine->B << 8) + machine->C;
            machine->A = machine->mem[tmp1];
            
            cycles = 7;
            break;
        }
        case 0x0B: {
            // DCX BC
            DCX(machine->B, machine->C)
            
            cycles = 5;
            break;
        }
        case 0x0C: {
            // INR C
            INR(machine->C)

            cycles = 5;
            break;
        }
        case 0x0D: {
            // DCR C
            DCR(machine->C)
            
            cycles = 5;
            break;
        }
        case 0x0E: {
            // MVI C
            machine->C = machine->mem[currentProgramCounter+1];
            
            instructionLength = 2;
            cycles = 7;
            break;
        }
        case 0x0F: {
            // RRC A - rotate right
            
            machine->carryFlag = machine->A & 1;
            machine->A = machine->A >> 1;
            machine->A += machine->carryFlag << 7;
            
            instructionLength = 1;
            cycles = 4;
            break;
        }
        case 0x11: {
            // LXI DE - load immediate to register pair
            
            machine->D = machine->mem[currentProgramCounter+2];
            machine->E = machine->mem[currentProgramCounter+1];
            
            instructionLength = 3;
            cycles = 10;
            break;
        }
        case 0x12: {
            // STAX DE - store accumulator A through register pair DE
            tmp1 = GET_REGISTER_PAIR(machine->D, machine->E);
            machine->mem[tmp1] = machine->A;
            
            cycles = 7;
            break;
        }
        case 0x13: {
            // INX DE
            INX(machine->D, machine->E)
            
            cycles = 5;
            break;
        }
        case 0x14: {
            // INR D
            INR(machine->D)
            
            cycles = 5;
            break;
        }
        case 0x15: {
            // DCR D
            DCR(machine->D)
            
            cycles = 5;
            break;
        }
        case 0x16: {
            // MVI D
            machine->D = machine->mem[currentProgramCounter+1];
            
            instructionLength = 2;
            cycles = 7;
            break;
        }
        case 0x17: {
            // RAL - rotate A through carry bit
            
            tmp1 = machine->carryFlag;
            machine->carryFlag = machine->A >> 7;
            machine->A = machine->A << 1;
            machine->A += tmp1;
            
            cycles = 4;
            break;
        }
        case 0x19: {
            // DAD DE
            DAD(machine->D, machine->E)
            
            cycles = 10;
            break;
        }
        case 0x1A: {
            // LDAX DE - load A through DE
            tmp1 = (machine->D << 8) + machine->E;
            machine->A = machine->mem[tmp1];
            
            cycles = 7;
            break;
        }
        case 0x1B: {
            // DCX DE
            DCX(machine->D, machine->E)
            
            cycles = 5;
            break;
        }
        case 0x1C: {
            // INR E
            INR(machine->E)
            
            cycles = 5;
            break;
        }
        case 0x1D: {
            // DCR E
            DCR(machine->E)
            
            cycles = 5;
            break;
        }
        case 0x1E: {
            // MVI E
            
            machine->E = machine->mem[currentProgramCounter+1];
            
            instructionLength = 2;
            cycles = 7;
            break;
        }
        case 0x1F: {
            // RAR - rotate right through carry
            
            tmp1 = machine->carryFlag;
            machine->carryFlag = machine->A & 1;
            machine->A = machine->A >> 1;
            machine->A += tmp1 << 7;
            
            cycles = 4;
            break;
        }
        case 0x21: {
            // LXI HL - load immediate to register pair HL
            
            tmp1 = READ_16BIT_IMMEDIATE;
            SET_REGISTER_PAIR(machine->H, machine->L, tmp1);
            
            instructionLength = 3;
            cycles = 10;
            break;
        }
        case 0x22: {
            // SHLD - store HL to immediate address
            tmp1 = READ_16BIT_IMMEDIATE;
            machine->mem[tmp1] = machine->L;
            machine->mem[tmp1+1] = machine->H;
            
            instructionLength = 3;
            cycles = 16;
            break;
        }
        case 0x23: {
            // INX HL
            INX(machine->H, machine->L)
            
            cycles = 5;
            break;
        }
        case 0x24: {
            // INR H
            INR(machine->H)
            
            cycles = 5;
            break;
        }
        case 0x25: {
            // DCR H
            DCR(machine->H)
            
            cycles = 5;
            break;
        }
        case 0x26: {
            // MVI H
            machine->H = machine->mem[currentProgramCounter+1];
            
            instructionLength = 2;
            cycles = 7;
            break;
        }
        case 0x27: {
            // DAA - Decimal Adjust Accumulator
            
            uint8_t add = 0;
            uint8_t carry = machine->carryFlag;
            uint8_t lo = machine->A & 0x0F;
            if(lo > 9 || machine->auxCarryFlag) {
                add = 6;
            }

            uint8_t hi = machine->A >> 4;
            if( hi > 9 || (hi == 9 && lo > 9) || carry) {
                add += 6 << 4;
                carry = 1;
            }
            
            ADD(add)
            machine->carryFlag = carry;
            
            cycles = 4;
            break;
        }
        case 0x29: {
            // DAD HL - add register pair to HL
            DAD(machine->H, machine->L)

            cycles = 10;
            break;
        }
        case 0x2A: {
            // LHLD HL - load HL from immediate address
    
            tmp1 = READ_16BIT_IMMEDIATE;
            machine->L = machine->mem[tmp1];
            machine->H = machine->mem[tmp1+1];
            
            instructionLength = 3;
            cycles = 16;
            break;
        }
        case 0x2B: {
            // DCX HL
            DCX(machine->H, machine->L)
            
            cycles = 5;
            break;
        }
        case 0x2C: {
            // INR L
            INR(machine->L)
            
            cycles = 5;
            break;
        }
        case 0x2D: {
            // DCR L
            DCR(machine->L)
            
            cycles = 5;
            break;
        }
        case 0x2E: {
            // MVI L
            machine->L = machine->mem[currentProgramCounter+1];
            
            instructionLength = 2;
            cycles = 7;
            break;
        }
        case 0x2F: {
            // CMA
            machine->A = ~ machine->A;
            
            cycles = 4;
            break;
        }
        case 0x31: {
            // LXI SP
            machine->stackPointer = READ_16BIT_IMMEDIATE;
            
            instructionLength = 3;
            cycles = 10;
            break;
        }
        case 0x32: {
            // STA - store A direct
            
            tmp1 = READ_16BIT_IMMEDIATE;
            machine->mem[tmp1] = machine->A;
            
            instructionLength = 3;
            cycles = 13;
            break;
        }
        case 0x33: {
            // INX SP
            machine->stackPointer = (uint16_t) machine->stackPointer + 1;
            
            cycles = 5;
            break;
        }
        case 0x34: {
            // INR M
            INR(machine->mem[memoryAddressRegister])
            
            cycles = 10;
            break;
        }
        case 0x35: {
            // DCR M
            DCR(machine->mem[memoryAddressRegister])
            
            cycles = 10;
            break;
        }
        case 0x36: {
            // MVI M
            machine->mem[memoryAddressRegister] = machine->mem[currentProgramCounter+1];
            
            instructionLength = 2;
            cycles = 10;
            break;
        }
        case 0x37: {
            // STC
            machine->carryFlag = 1;
            
            cycles = 4;
            break;
        }
        case 0x39: {
            // DAD SP - add register pair to HL
            
            tmp1 = machine->L + (machine->stackPointer & 0x00FF);
            tmp2 = machine->H + (machine->stackPointer >> 8) + (tmp1 >> 8);
            machine->L = tmp1 & 0xFF;
            machine->H = tmp2 & 0xFF;
            CARRY(tmp2)

            cycles = 10;
            break;
        }
        case 0x3A: {
            // LDA - load A direct
            tmp1 = READ_16BIT_IMMEDIATE;
            machine->A = machine->mem[tmp1];
            
            instructionLength = 3;
            cycles = 13;
            break;
        }
        case 0x3B: {
            // DCX SP
            machine->stackPointer = (uint16_t) machine->stackPointer-1;
            
            cycles = 5;
            break;
        }
        case 0x3C: {
            // INR A
            INR(machine->A)
            
            cycles = 5;
            break;
        }
        case 0x3D: {
            // DCR A
            DCR(machine->A)
            
            cycles = 5;
            break;
        }
        case 0x3E: {
            // MVI A
            machine->A = machine->mem[currentProgramCounter + 1];
            
            instructionLength = 2;
            cycles = 7;
            break;
        }
        case 0x3F: {
            // CMC (Complement Carry)
            machine->carryFlag = ~ machine->carryFlag;
            
            cycles = 4;
            break;
        }
        case 0x40: {
            // MOV B,B
            
            cycles = 5;
            break;
        }
        case 0x41: {
            // MOV B,C
            machine->B = machine->C;
            
            cycles = 5;
            break;
        }
        case 0x42: {
            // MOV B,D
            machine->B = machine->D;
            
            cycles = 5;
            break;
        }
        case 0x43: {
            // MOV B,E
            machine->B = machine->E;
            
            cycles = 5;
            break;
        }
        case 0x44: {
            // MOV B,H
            machine->B = machine->H;
            
            cycles = 5;
            break;
        }
        case 0x45: {
            // MOV B,L
            machine->B = machine->L;
            
            cycles = 5;
            break;
        }
        case 0x46: {
            // MOV B,M
            machine->B = machine->mem[memoryAddressRegister];
            
            cycles = 7;
            break;
        }
        case 0x47: {
            // MOV B,A
            machine->B = machine->A;
            
            cycles = 5;
            break;
        }
        case 0x48: {
            // MOV C,B
            machine->C = machine->B;
            
            cycles = 5;
            break;
        }
        case 0x49: {
            // MOV C,C
            
            cycles = 5;
            break;
        }
        case 0x4A: {
            // MOV C,D
            machine->C = machine->D;
            
            cycles = 5;
            break;
        }
        case 0x4B: {
            // MOV C,E
            machine->C = machine->E;
            
            cycles = 5;
            break;
        }
        case 0x4C: {
            // MOV C,H
            machine->C = machine->H;
            
            cycles = 5;
            break;
        }
        case 0x4D: {
            // MOV C,L
            machine->C = machine->L;
            
            cycles = 5;
            break;
        }
        case 0x4E: {
            // MOV C,M
            machine->C = machine->mem[memoryAddressRegister];
            
            cycles = 7;
            break;
        }
        case 0x4F: {
            // MOV C,A
            machine->C = machine->A;
            
            cycles = 5;
            break;
        }
        case 0x50: {
            // MOV D,B
            machine->D = machine->B;
            
            cycles = 5;
            break;
        }
        case 0x51: {
            // MOV D,C
            machine->D = machine->C;
            
            cycles = 5;
            break;
        }
        case 0x52: {
            // MOV D,D
            
            cycles = 5;
            break;
        }
        case 0x53: {
            // MOV D,E
            machine->D = machine->E;
            
            cycles = 5;
            break;
        }
        case 0x54: {
            // MOV D,H
            machine->D = machine->H;
            
            cycles = 5;
            break;
        }
        case 0x55: {
            // MOV D,L
            machine->D = machine->L;
            
            cycles = 5;
            break;
        }
        case 0x56: {
            // MOV D,M
            machine->D = machine->mem[memoryAddressRegister];
            
            cycles = 7;
            break;
        }
        case 0x57: {
            // MOV D,A
            machine->D = machine->A;

            cycles = 5;
            break;
        }
        case 0x58: {
            // MOV E,B
            machine->E = machine->B;

            cycles = 5;
            break;
        }
        case 0x59: {
            // MOV E,C
            machine->E = machine->C;
            
            cycles = 5;
            break;
        }
        case 0x5A: {
            // MOV E,D
            machine->E = machine->D;
            
            cycles = 5;
            break;
        }
        case 0x5B: {
            // MOV E,E
            
            cycles = 5;
            break;
        }
        case 0x5C: {
            // MOV E,H
            machine->E = machine->H;
            
            cycles = 5;
            break;
        }
        case 0x5D: {
            // MOV E,L
            machine->E = machine->L;
            
            cycles = 5;
            break;
        }
        case 0x5E: {
            // MOV E,M
            machine->E = machine->mem[memoryAddressRegister];
            
            cycles = 7;
            break;
        }
        case 0x5F: {
            // MOV E,A
            machine->E = machine->A;
            
            cycles = 5;
            break;
        }
        case 0x60: {
            // MOV H,B
            machine->H = machine->B;
            
            cycles = 5;
            break;
        }
        case 0x61: {
            // MOV H,C
            machine->H = machine->C;
            
            cycles = 5;
            break;
        }
        case 0x62: {
            // MOV H,D
            machine->H = machine->D;
            
            cycles = 5;
            break;
        }
        case 0x63: {
            // MOV H,E
            machine->H = machine->E;
            
            cycles = 5;
            break;
        }
        case 0x64: {
            // MOV H,H
            
            cycles = 5;
            break;
        }
        case 0x65: {
            // MOV H,L
            machine->H = machine->L;
            
            cycles = 5;
            break;
        }
        case 0x66: {
            // MOV H,M
            machine->H = machine->mem[memoryAddressRegister];
            
            cycles = 7;
            break;
        }
        case 0x67: {
            // MOV H,A
            machine->H = machine->A;
            
            cycles = 5;
            break;
        }
        case 0x68: {
            // MOV L,B
            machine->L = machine->B;
            
            cycles = 5;
            break;
        }
        case 0x69: {
            // MOV L,C
            machine->L = machine->C;
            
            cycles = 5;
            break;
        }
        case 0x6A: {
            // MOV L,D
            machine->L = machine->D;
            
            cycles = 5;
            break;
        }
        case 0x6B: {
            // MOV L,E
            machine->L = machine->E;
            
            cycles = 5;
            break;
        }
        case 0x6C: {
            // MOV L,H
            machine->L = machine->H;
            
            cycles = 5;
            break;
        }
        case 0x6D: {
            // MOV L,L
            
            cycles = 5;
            break;
        }
        case 0x6E: {
            // MOV L,M
            machine->L = machine->mem[memoryAddressRegister];
            
            cycles = 7;
            break;
        }
        case 0x6F: {
            // MOV L,A
            machine->L = machine->A;
            
            cycles = 5;
            break;
        }
        case 0x70: {
            // MOV M,B
            machine->mem[memoryAddressRegister] = machine->B;
            
            cycles = 7;
            break;
        }
        case 0x71: {
            // MOV M,C
            machine->mem[memoryAddressRegister] = machine->C;
            
            cycles = 7;
            break;
        }
        case 0x72: {
            // MOV M,D
            machine->mem[memoryAddressRegister] = machine->D;
            
            cycles = 7;
            break;
        }
        case 0x73: {
            // MOV M,E
            machine->mem[memoryAddressRegister] = machine->E;
            
            cycles = 7;
            break;
        }
        case 0x74: {
            // MOV M,H
            machine->mem[memoryAddressRegister] = machine->H;
            
            cycles = 7;
            break;
        }
        case 0x75: {
            // MOV M,L
            machine->mem[memoryAddressRegister] = machine->L;
            
            cycles = 7;
            break;
        }
        case 0x76: {
            // HLT
            
            haltSignal = true;
            cycles = 4;
            break;
        }
        case 0x77: {
            // MOV M,A
            machine->mem[memoryAddressRegister] = machine->A;
            
            cycles = 7;
            break;
        }
        case 0x78: {
            // MOV A,B
            machine->A = machine->B;
            
            cycles = 5;
            break;
        }
        case 0x79: {
            // MOV A,C
            machine->A = machine->C;
            
            cycles = 5;
            break;
        }
        case 0x7A: {
            // MOV A,D
            machine->A = machine->D;
            
            cycles = 5;
            break;
        }
        case 0x7B: {
            // MOV A,E
            machine->A = machine->E;
            
            cycles = 5;
            break;
        }
        case 0x7C: {
            // MOV A,H
            machine->A = machine->H;
            
            cycles = 5;
            break;
        }
        case 0x7D: {
            // MOV A,L
            machine->A = machine->L;
            
            cycles = 5;
            break;
        }
        case 0x7E: {
            // MOV A,M
            machine->A = machine->mem[memoryAddressRegister];
            
            cycles = 7;
            break;
        }
        case 0x7F: {
            // MOV A,A
            
            cycles = 5;
            break;
        }
        case 0x80: {
            // ADD B
            ADD(machine->B);
            
            cycles = 4;
            break;
        }
        case 0x81: {
            // ADD C
            ADD(machine->C)
            
            cycles = 4;
            break;
        }
        case 0x82: {
            // ADD D
            ADD(machine->D)
            
            cycles = 4;
            break;
        }
        case 0x83: {
            // ADD E
            ADD(machine->E)
            
            cycles = 4;
            break;
        }
        case 0x84: {
            // ADD H
            ADD(machine->H)
            
            cycles = 4;
            break;
        }
        case 0x85: {
            // ADD L
            ADD(machine->L)
            
            cycles = 4;
            break;
        }
        case 0x86: {
            // ADD M
            ADD(machine->mem[memoryAddressRegister])
            
            cycles = 7;
            break;
        }
        case 0x87: {
            // ADD A
            ADD(machine->A)
            
            cycles = 4;
            break;
        }
        case 0x88: {
            // ADC B
            ADC(machine->B)
            
            cycles = 4;
            break;
        }
        case 0x89: {
            // ADC C
            ADC(machine->C)
            
            cycles = 4;
            break;
        }
        case 0x8A: {
            // ADC D
            ADC(machine->D)
            
            cycles = 4;
            break;
        }
        case 0x8B: {
            // ADC E
            ADC(machine->E)
            
            cycles = 4;
            break;
        }
        case 0x8C: {
            // ADC H
            ADC(machine->H)
            
            cycles = 4;
            break;
        }
        case 0x8D: {
            // ADC L
            ADC(machine->L)
            
            cycles = 4;
            break;
        }
        case 0x8E: {
            // ADC M
            ADC(machine->mem[memoryAddressRegister])
            
            cycles = 7;
            break;
        }
        case 0x8F: {
            // ADC A
            ADC(machine->A)
            
            cycles = 4;
            break;
        }
        case 0x90: {
            // SUB B
            SUB(machine->B)
            
            cycles = 4;
            break;
        }
        case 0x91: {
            // SUB C
            SUB(machine->C)
            
            cycles = 4;
            break;
        }
        case 0x92: {
            // SUB D
            SUB(machine->D)
            
            cycles = 4;
            break;
        }
        case 0x93: {
            // SUB E
            SUB(machine->E)
            
            cycles = 4;
            break;
        }
        case 0x94: {
            // SUB H
            SUB(machine->H)
            
            cycles = 4;
            break;
        }
        case 0x95: {
            // SUB L
            SUB(machine->L)
            
            cycles = 4;
            break;
        }
        case 0x96: {
            // SUB M
            SUB(machine->mem[memoryAddressRegister])
            
            cycles = 7;
            break;
        }
        case 0x97: {
            // SUB A
            machine->A = 0;
            machine->carryFlag = 0;
            machine->auxCarryFlag = 1;
            machine->parityFlag = 1;
            machine->zeroFlag = 1;
            machine->signFlag = 0;
            
            cycles = 4;
            break;
        }
        case 0x98: { // SBB - subtract with borrow
            // SBB B
            SBB(machine->B)
            
            cycles = 4;
            break;
        }
        case 0x99: {
            // SBB C
            SBB(machine->C)
            
            cycles = 4;
            break;
        }
        case 0x9A: {
            // SBB D
            SBB(machine->D)
            
            cycles = 4;
            break;
        }
        case 0x9B: {
            // SBB E
            SBB(machine->E)
            
            cycles = 4;
            break;
        }
        case 0x9C: {
            // SBB H
            SBB(machine->H)
            
            cycles = 4;
            break;
        }
        case 0x9D: {
            // SBB L
            SBB(machine->L)
            
            cycles = 4;
            break;
        }
        case 0x9E: {
            // SBB M
            SBB(machine->mem[memoryAddressRegister])
            
            cycles = 7;
            break;
        }
        case 0x9F: {
            // SBB A
            SBB(machine->A)
            
            cycles = 4;
            break;
        }
        case 0xA0: {
            // ANA B
            ANA(machine->B)
            
            cycles = 4;
            break;
        }
        case 0xA1: {
            // ANA C
            ANA(machine->C)
            
            cycles = 4;
            break;
        }
        case 0xA2: {
            // ANA D
            ANA(machine->D)
            
            cycles = 4;
            break;
        }
        case 0xA3: {
            // ANA E
            ANA(machine->E)
            
            cycles = 4;
            break;
        }
        case 0xA4: {
            // ANA H
            ANA(machine->H)
            
            cycles = 4;
            break;
        }
        case 0xA5: {
            // ANA L
            ANA(machine->L)
            
            cycles = 4;
            break;
        }
        case 0xA6: {
            // ANA M
            ANA(machine->mem[memoryAddressRegister])
            
            cycles = 7;
            break;
        }
        case 0xA7: {
            // ANA A
            ANA(machine->A)
            
            cycles = 4;
            break;
        }
        case 0xA8: {
            // XOR B
            XOR(machine->B)
            
            cycles = 4;
            break;
        }
        case 0xA9: {
            // XOR C
            XOR(machine->C)
            
            cycles = 4;
            break;
        }
        case 0xAA: {
            // XOR D
            XOR(machine->D)
            
            cycles = 4;
            break;
        }
        case 0xAB: {
            // XOR E
            XOR(machine->E)
            
            cycles = 4;
            break;
        }
        case 0xAC: {
            // XOR H
            XOR(machine->H)
            
            cycles = 4;
            break;
        }
        case 0xAD: {
            // XOR L
            XOR(machine->L)
            
            cycles = 4;
            break;
        }
        case 0xAE: {
            // XOR M
            XOR(machine->mem[memoryAddressRegister])
            
            cycles = 7;
            break;
        }
        case 0xAF: {
            // XOR A
            XOR(machine->A)
            
            cycles = 4;
            break;
        }
        case 0xB0: {
            // ORA B
            ORA(machine->B)
            
            cycles = 4;
            break;
        }
        case 0xB1: {
            // ORA C
            ORA(machine->C)
            
            cycles = 4;
            break;
        }
        case 0xB2: {
            // ORA D
            ORA(machine->D)
            
            cycles = 4;
            break;
        }
        case 0xB3: {
            // ORA E
            ORA(machine->E)
            
            cycles = 4;
            break;
        }
        case 0xB4: {
            // ORA H
            ORA(machine->H)
            
            cycles = 4;
            break;
        }
        case 0xB5: {
            // ORA L
            ORA(machine->L)
            
            cycles = 4;
            break;
        }
        case 0xB6: {
            // ORA M
            ORA(machine->mem[memoryAddressRegister])
            
            cycles = 7;
            break;
        }
        case 0xB7: {
            // ORA A
            ORA(machine->A)
            
            cycles = 4;
            break;
        }
        case 0xB8: {
            // CMP B
            CMP(machine->B)
            
            cycles = 4;
            break;
        }
        case 0xB9: {
            // CMP C
            CMP(machine->C)
            
            cycles = 4;
            break;
        }
        case 0xBA: {
            // CMP D
            CMP(machine->D)
            
            cycles = 4;
            break;
        }
        case 0xBB: {
            // CMP E
            CMP(machine->E)
            
            cycles = 4;
            break;
        }
        case 0xBC: {
            // CMP H
            CMP(machine->H)
            
            cycles = 4;
            break;
        }
        case 0xBD: {
            // CMP L
            CMP(machine->L)
            
            cycles = 4;
            break;
        }
        case 0xBE: {
            // CMP M
            CMP(machine->mem[memoryAddressRegister])
            
            cycles = 7;
            break;
        }
        case 0xBF: {
            // CMP A
            machine->zeroFlag = 1; 
            machine->carryFlag = 0;
            machine->signFlag = 0;
            machine->parityFlag = 1;
            machine->auxCarryFlag = 1;
            
            cycles = 4;
            break;
        }
        case 0xC0: {
            // RNZ - return if not zero
            CONDITIONAL_RETURN(machine->zeroFlag == 0)
            break;
        }
        case 0xC1: {
            // POP BC
            
            machine->C = machine->mem[machine->stackPointer];
            machine->B = machine->mem[machine->stackPointer + 1];
            machine->stackPointer += 2;

            cycles = 11;
            break;
        }
        case 0xC2: {
            // JNZ - jump if not zero
            CONDITIONAL_JUMP(machine->zeroFlag == 0)
            break;
        }
        case 0xC3:
        case 0xCB: {
            // JMP
            CONDITIONAL_JUMP(true)
            break;
        }
        case 0xC4: {
            // CNZ - call if not zero
            CONDITIONAL_CALL(machine->zeroFlag == 0)
            break;
        }
        case 0xC5: {
            // PUSH BC
            
            machine->mem[machine->stackPointer - 1] = machine->B;
            machine->mem[machine->stackPointer - 2] = machine->C;
            machine->stackPointer -= 2;

            cycles = 11;
            break;
        }
        case 0xC6: {
            // ADI - add immediate to A

            ADD(machine->mem[currentProgramCounter+1])
            
            instructionLength = 2;
            cycles = 7;
            break;
        }
        case 0xC7: {
            // RST 0
            
            RST(0)
            break;
        }
        case 0xC8: {
            // RZ - return if zero
            CONDITIONAL_RETURN(machine->zeroFlag == 1)
            break;
        }
        case 0xC9:
        case 0xD9: {
            // RET

            RETURN
            cycles = 10;
            break;
        }
        case 0xCA: {
            // JZ - jump if zero
            CONDITIONAL_JUMP(machine->zeroFlag == 1)
            break;
        }
        case 0xCC: {
            // CZ - call if zero
            CONDITIONAL_CALL(machine->zeroFlag == 1)
            break;
        }
        case 0xCD:
        case 0xDD:
        case 0xED:
        case 0xFD: {
            // CALL
            
            tmp1 = READ_16BIT_IMMEDIATE;
            CALL_IMMEDIATE(tmp1)
            break;
        }
        case 0xCE: {
            // ACI (ADI w/ carry)

            ADC(machine->mem[currentProgramCounter+1]);
            
            instructionLength = 2;
            cycles = 7;
            break;
        }
        case 0xCF: {
            // RST 1
            
            RST(1)
            break;
        }
        case 0xD0: {
            // RNC - return if not carry
            CONDITIONAL_RETURN(machine->carryFlag == 0)
            break;
        }
        case 0xD1: {
            // POP DE
            
            machine->E = machine->mem[ machine->stackPointer ];
            machine->D = machine->mem[ machine->stackPointer + 1 ];
            machine->stackPointer += 2;
            
            cycles = 11;
            break;
        }
        case 0xD2: {
            // JNC - jump if not carry
            CONDITIONAL_JUMP(machine->carryFlag == 0)
            break;
        }
        case 0xD3: {
            // OUT
            
            instructionLength = 2;
            cycles = 10;
            break;
        }
        case 0xD4: {
            // CNC - call if not carry
            CONDITIONAL_CALL(machine->carryFlag == 0)
            break;
        }
        case 0xD5: {
            // PUSH DE

            machine->mem[ machine->stackPointer - 2 ] = machine->E;
            machine->mem[ machine->stackPointer - 1 ] = machine->D;
            
            machine->stackPointer -= 2;
            cycles = 11;
            break;
        }
        case 0xD6: {
            // SUI

            SUB(machine->mem[currentProgramCounter+1]);
            
            instructionLength = 2;
            cycles = 7;
            break;
        }
        case 0xD7: {
            // RST 2
            
            RST(2)
            break;
        }
        case 0xD8: {
            // RC - return if carry
            CONDITIONAL_RETURN(machine->carryFlag == 1)
            break;
        }
        case 0xDA: {
            // JC - jump if carry
            CONDITIONAL_JUMP(machine->carryFlag == 1)
            break;
        }
        case 0xDB: {
            // IN
            
            instructionLength = 2;
            cycles = 10;
            break;
        }
        case 0xDC: {
            // CC - call if carry
            CONDITIONAL_CALL(machine->carryFlag == 1)
            break;
        }
        case 0xDE: {
            // SBI - Subtract Immediate with Borrow

            SBB(machine->mem[currentProgramCounter+1])
            
            instructionLength = 2;
            cycles = 7;
            break;
        }
        case 0xDF: {
            // RST 3
            
            RST(3)
            break;
        }
        case 0xE0: {
            // RPO - return if parity odd
            CONDITIONAL_RETURN(machine->parityFlag == 0)
            break;
        }
        case 0xE1: {
            // POP HL

            machine->L = machine->mem[machine->stackPointer];
            machine->H = machine->mem[machine->stackPointer + 1];
            machine->stackPointer += 2;

            cycles = 11;
            break;
        }
        case 0xE2: {
            // JPO - jump if parity odd
            CONDITIONAL_JUMP(machine->parityFlag == 0)
            break;
        }
        case 0xE3: {
            // XTHL: swap HL with word [SP]
            //         i.e. [SP] <-> L; [SP+1] <-> H;
            
            tmp1 = (machine->H << 8) + machine->L;
            machine->L = machine->mem[machine->stackPointer];
            machine->H = machine->mem[machine->stackPointer + 1];
            WRITE_16BIT_TO_MEM(machine->stackPointer, tmp1);
            
            cycles = 18;
            break;
        }
        case 0xE4: {
            // CPO - call if parity odd
            CONDITIONAL_CALL(machine->parityFlag == 0)
            break;
        }
        case 0xE5: {
            // PUSH HL

            machine->mem[machine->stackPointer - 2] = machine->L;
            machine->mem[machine->stackPointer - 1] = machine->H;
            machine->stackPointer -= 2;
            
            cycles = 11;
            break;
        }
        case 0xE6: {
            // ANI - logical AND immediate with A
            
            ANA(machine->mem[currentProgramCounter+1])
            
            instructionLength = 2;
            cycles = 7;
            break;
        }
        case 0xE7: {
            // RST 4
            
            RST(4)
            break;
        }
        case 0xE8: {
            // RPE - return if parity even
            CONDITIONAL_RETURN(machine->parityFlag == 1)
            break;
        }
        case 0xE9: {
            // PCHL - jump to [HL]
            
            machine->programCounter = (machine->H << 8) + machine->L;
            instructionLength = 0;
            
            cycles = 5;
            break;
        }
        case 0xEA: {
            // JPE - jump if parity even
            CONDITIONAL_JUMP(machine->parityFlag == 1)
            break;
        }
        case 0xEB: {
            // XCHG - exchange DE and HL
            
            tmp1 = (machine->D << 8) + machine->E;
            
            machine->D = machine->H;
            machine->E = machine->L;
            
            machine->H = tmp1 >> 8;
            machine->L = tmp1 & 0xFF;
            
            cycles = 4;
            break;
        }
        case 0xEC: {
            // CPE - call if parity even
            CONDITIONAL_CALL(machine->parityFlag == 1)
            break;
        }
        case 0xEE: {
            // XRI - logical XOR immediate with A

            XOR(machine->mem[currentProgramCounter+1]);
            
            instructionLength = 2;
            cycles = 7;
            break;
        }
        case 0xEF: {
            // RST 5
            
            RST(5)
            break;
        }
        case 0xF0: {
            // RP - return if plus (positive)
            CONDITIONAL_RETURN(machine->signFlag == 0)
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
            cycles = 10;
            break;
        }
        case 0xF2: {
            // JP - jump if plus (positive)
            CONDITIONAL_JUMP(machine->signFlag == 0)
            break;
        }
        case 0xF3: {
            // DI - disable interrupts

            machine->interrupts = 0;
            cycles = 4;
            break;
        }
        case 0xF4: {
            // CP - call if plus (positive)
            CONDITIONAL_CALL(machine->signFlag == 0)
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
            
            cycles = 11;
            break;
        }
        case 0xF6: {
            // ORI - logical OR immediate with A

            ORA(machine->mem[currentProgramCounter+1])

            instructionLength = 2;
            cycles = 7;
            break;
        }
        case 0xF7: {
            // RST 6
            
            RST(6)
            break;
        }
        case 0xF8: {
            // RM - return if minus (negative)
            CONDITIONAL_RETURN(machine->signFlag == 1)
            break;
        }
        case 0xF9: {
            // SPHL - set SP to HL
            
            machine->stackPointer = (machine->H << 8) + machine->L;
            cycles = 5;
            break;
        }
        case 0xFA: {
            // JM - jump if minus (negative)
            CONDITIONAL_JUMP(machine->signFlag == 1)
            break;
        }
        case 0xFB: {
            // EI - enable interrupts

            machine->interrupts = 1;
            cycles = 4;
            break;
        }
        case 0xFC: {
            // CM - call if minus (negative)
            CONDITIONAL_CALL(machine->signFlag == 1)
            break;
        }
        case 0xFE: {
            // CPI - compare immediate with A
            
            CMP(machine->mem[currentProgramCounter+1]);
            
            instructionLength = 2;
            cycles = 7;
            break;
        }
        case 0xFF: {
            // RST 7
            
            RST(7)
            break;
        }
    }
    
    machine->programCounter += instructionLength;
    return (haltSignal) ? (I8080_HALT) : cycles;
}
