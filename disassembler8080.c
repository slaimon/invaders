#include <string.h>
#include "disassembler8080.h"

// raccoglie tutte le info sulla istruzione fornita e le restituisce senza stamparle
instruction8080_t disassemble_instruction( uint8_t* mem , unsigned int addr ) {
    instruction8080_t data;
    
    data.opcode = mem[addr];
    
    data.position = addr;
    data.num_inputRegisters = 0;     //
    data.num_inputValues = 0;        //
    data.instructionLength = 1;      // default values
    data.immediate = true;           //
    
    switch (data.opcode) {
        case 0x00: {
            strcpy(data.mnemonic,"NOP");
            data.instructionLength = 1;
            break;
        }
        case 0x01: {
            strcpy(data.mnemonic, "LXI");
            strcpy(data.inputRegisters, "B");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            
            data.instructionLength = 3;
            data.num_inputRegisters = 1;
            data.num_inputValues = 2;
            break;
        }
        case 0x02: {
            strcpy(data.mnemonic, "STAX");
            strcpy(data.inputRegisters, "B");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x03: {
            strcpy(data.mnemonic, "INX");
            strcpy(data.inputRegisters, "B");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x04: {
            strcpy(data.mnemonic, "INR");
            strcpy(data.inputRegisters, "B");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x05: {
            strcpy(data.mnemonic, "DCR");
            strcpy(data.inputRegisters, "B");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x06: {
            strcpy(data.mnemonic, "MVI");
            strcpy(data.inputRegisters, "B");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 1;
            data.num_inputValues = 1;
            break;
        }
        case 0x07: {
            strcpy(data.mnemonic,"RLC");
            data.instructionLength = 1;
            break;
        }
        case 0x08: {
            strcpy(data.mnemonic,"NOP");
            data.instructionLength = 1;
            break;
        }
        case 0x09: {
            strcpy(data.mnemonic, "DAD");
            strcpy(data.inputRegisters, "B");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x0A: {
            strcpy(data.mnemonic, "LDAX");
            strcpy(data.inputRegisters, "B");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x0B: {
            strcpy(data.mnemonic, "DCX");
            strcpy(data.inputRegisters, "B");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x0C: {
            strcpy(data.mnemonic, "INR");
            strcpy(data.inputRegisters, "C");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x0D: {
            strcpy(data.mnemonic, "DCR");
            strcpy(data.inputRegisters, "C");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x0E: {
            strcpy(data.mnemonic, "MVI");
            strcpy(data.inputRegisters, "C");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 1;
            data.num_inputValues = 1;
            break;
        }
        case 0x0F: {
            strcpy(data.mnemonic, "RRC");
            
            data.instructionLength = 1;
            break;
        }
        case 0x10: {
            strcpy(data.mnemonic, "NOP");
            
            data.instructionLength = 1;
            break;
        }
        case 0x11: {
            strcpy(data.mnemonic, "LXI");
            strcpy(data.inputRegisters, "D");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            
            data.instructionLength = 3;
            data.num_inputRegisters = 1;
            data.num_inputValues = 2;
            break;
        }
        case 0x12: {
            strcpy(data.mnemonic, "STAX");
            strcpy(data.inputRegisters, "D");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x13: {
            strcpy(data.mnemonic, "INX");
            strcpy(data.inputRegisters, "D");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x14: {
            strcpy(data.mnemonic, "INR");
            strcpy(data.inputRegisters, "D");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x15: {
            strcpy(data.mnemonic, "DCR");
            strcpy(data.inputRegisters, "D");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x16: {
            strcpy(data.mnemonic, "MVI");
            strcpy(data.inputRegisters, "D");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 1;
            data.num_inputValues = 1;
            break;
        }
        case 0x17: {
            strcpy(data.mnemonic,"RAL");
            data.instructionLength = 1;
            break;
        }
        case 0x18: {
            strcpy(data.mnemonic,"NOP");
            data.instructionLength = 1;
            break;
        }
        case 0x19: {
            strcpy(data.mnemonic, "DAD");
            strcpy(data.inputRegisters, "D");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x1A: {
            strcpy(data.mnemonic, "LDAX");
            strcpy(data.inputRegisters, "D");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x1B: {
            strcpy(data.mnemonic, "DCX");
            strcpy(data.inputRegisters, "E");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x1C: {
            strcpy(data.mnemonic, "INR");
            strcpy(data.inputRegisters, "E");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x1D: {
            strcpy(data.mnemonic, "DCR");
            strcpy(data.inputRegisters, "E");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x1E: {
            strcpy(data.mnemonic, "MVI");
            strcpy(data.inputRegisters, "E");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 1;
            data.num_inputValues = 1;
            break;
        }
        case 0x1F: {
            strcpy(data.mnemonic, "RAR");
            
            data.instructionLength = 1;
            break;
        }
        case 0x20: {
            strcpy(data.mnemonic, "NOP");
            
            data.instructionLength = 1;
            break;
        }
        case 0x21: {
            strcpy(data.mnemonic, "LXI");
            strcpy(data.inputRegisters, "H");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            
            data.instructionLength = 3;
            data.num_inputRegisters = 1;
            data.num_inputValues = 2;
            break;
        }
        case 0x22: {
            strcpy(data.mnemonic, "SHLD");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputRegisters = 0;
            data.num_inputValues = 2;
            break;
        }
        case 0x23: {
            strcpy(data.mnemonic, "INX");
            strcpy(data.inputRegisters, "H");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x24: {
            strcpy(data.mnemonic, "INR");
            strcpy(data.inputRegisters, "H");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x25: {
            strcpy(data.mnemonic, "DCR");
            strcpy(data.inputRegisters, "H");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x26: {
            strcpy(data.mnemonic, "MVI");
            strcpy(data.inputRegisters, "H");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 1;
            data.num_inputValues = 1;
            break;
        }
        case 0x27: {
            strcpy(data.mnemonic,"DAA");
            data.instructionLength = 1;
            break;
        }
        case 0x28: {
            strcpy(data.mnemonic,"NOP");
            data.instructionLength = 1;
            break;
        }
        case 0x29: {
            strcpy(data.mnemonic, "DAD");
            strcpy(data.inputRegisters, "H");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x2A: {
            strcpy(data.mnemonic, "LHLD");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+1];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputRegisters = 0;
            data.num_inputValues = 2;
            break;
        }
        case 0x2B: {
            strcpy(data.mnemonic, "DCX");
            strcpy(data.inputRegisters, "H");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x2C: {
            strcpy(data.mnemonic, "INR");
            strcpy(data.inputRegisters, "L");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x2D: {
            strcpy(data.mnemonic, "DCR");
            strcpy(data.inputRegisters, "L");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x2E: {
            strcpy(data.mnemonic, "MVI");
            strcpy(data.inputRegisters, "L");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 1;
            data.num_inputValues = 1;
            break;
        }
        case 0x2F: {
            strcpy(data.mnemonic, "CMA");
            
            data.instructionLength = 1;
            break;
        }
        case 0x30: {
            strcpy(data.mnemonic, "NOP");
            
            data.instructionLength = 1;
            break;
        }
        case 0x31: {
            strcpy(data.mnemonic, "LXI");
            strcpy(data.inputRegisters, "SP");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            
            data.instructionLength = 3;
            data.num_inputRegisters = 1;
            data.num_inputValues = 2;
            break;
        }
        case 0x32: {
            strcpy(data.mnemonic, "STA");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputRegisters = 0;
            data.num_inputValues = 2;
            break;
        }
        case 0x33: {
            strcpy(data.mnemonic, "INX");
            strcpy(data.inputRegisters, "SP");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x34: {
            strcpy(data.mnemonic, "INR");
            strcpy(data.inputRegisters, "M");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x35: {
            strcpy(data.mnemonic, "DCR");
            strcpy(data.inputRegisters, "M");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x36: {
            strcpy(data.mnemonic, "MVI");
            strcpy(data.inputRegisters, "M");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 1;
            data.num_inputValues = 1;
            break;
        }
        case 0x37: {
            strcpy(data.mnemonic,"STC");
            
            data.instructionLength = 1;
            break;
        }
        case 0x38: {
            strcpy(data.mnemonic,"NOP");
            
            data.instructionLength = 1;
            break;
        }
        case 0x39: {
            strcpy(data.mnemonic, "DAD");
            strcpy(data.inputRegisters, "SP");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x3A: {
            strcpy(data.mnemonic, "LDA");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputRegisters = 0;
            data.num_inputValues = 2;
            break;
        }
        case 0x3B: {
            strcpy(data.mnemonic, "DCX");
            strcpy(data.inputRegisters, "SP");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x3C: {
            strcpy(data.mnemonic, "INR");
            strcpy(data.inputRegisters, "A");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x3D: {
            strcpy(data.mnemonic, "DCR");
            strcpy(data.inputRegisters, "A");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;
            break;
        }
        case 0x3E: {
            strcpy(data.mnemonic, "MVI");
            strcpy(data.inputRegisters, "A");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 1;
            data.num_inputValues = 1;
            break;
        }
        case 0x3F: {
            strcpy(data.mnemonic, "CMC");
            
            data.instructionLength = 1;
            break;
        }
        case 0x40: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "B,B");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x41: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "B,C");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x42: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "B,D");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x43: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "B,E");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x44: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "B,H");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x45: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "B,L");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x46: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "B,M");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x47: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "B,A");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x48: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "C,B");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x49: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "C,C");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x4A: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "C,D");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x4B: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "C,E");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x4C: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "C,H");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x4D: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "C,L");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x4E: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "C,M");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x4F: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "C,A");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x50: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "D,B");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x51: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "D,C");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x52: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "D,D");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x53: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "D,E");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x54: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "D,H");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x55: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "D,L");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x56: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "D,M");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x57: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "D,A");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x58: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "E,B");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x59: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "E,C");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x5A: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "E,D");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x5B: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "E,E");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x5C: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "E,H");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x5D: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "E,L");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x5E: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "E,M");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x5F: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "E,A");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x60: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "H,B");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x61: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "H,C");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x62: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "H,D");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x63: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "H,E");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x64: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "H,H");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x65: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "H,L");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x66: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "H,M");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x67: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "H,A");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x68: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "L,B");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x69: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "L,C");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x6A: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "L,D");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x6B: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "L,E");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x6C: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "L,H");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x6D: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "L,L");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x6E: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "L,M");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x6F: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "L,A");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x70: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "M,B");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x71: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "M,C");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x72: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "M,D");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x73: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "M,E");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x74: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "M,H");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x75: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "M,L");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x76: {
            strcpy(data.mnemonic, "HLT");
            
            data.instructionLength = 1;
            break;
        }
        case 0x77: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "M,A");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x78: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "A,B");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x79: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "A,C");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x7A: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "A,D");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x7B: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "A,E");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x7C: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "A,H");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x7D: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "A,L");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x7E: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "A,M");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x7F: {
            strcpy(data.mnemonic, "MOV");
            strcpy(data.inputRegisters, "A,A");
            
            data.num_inputRegisters = 2;
            break;
        }
        case 0x80: {
            strcpy(data.mnemonic, "ADD");
            strcpy(data.inputRegisters, "B");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x81: {
            strcpy(data.mnemonic, "ADD");
            strcpy(data.inputRegisters, "C");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x82: {
            strcpy(data.mnemonic, "ADD");
            strcpy(data.inputRegisters, "D");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x83: {
            strcpy(data.mnemonic, "ADD");
            strcpy(data.inputRegisters, "E");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x84: {
            strcpy(data.mnemonic, "ADD");
            strcpy(data.inputRegisters, "H");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x85: {
            strcpy(data.mnemonic, "ADD");
            strcpy(data.inputRegisters, "L");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x86: {
            strcpy(data.mnemonic, "ADD");
            strcpy(data.inputRegisters, "M");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x87: {
            strcpy(data.mnemonic, "ADD");
            strcpy(data.inputRegisters, "A");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x88: {
            strcpy(data.mnemonic, "ADC");
            strcpy(data.inputRegisters, "B");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x89: {
            strcpy(data.mnemonic, "ADC");
            strcpy(data.inputRegisters, "C");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x8A: {
            strcpy(data.mnemonic, "ADC");
            strcpy(data.inputRegisters, "D");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x8B: {
            strcpy(data.mnemonic, "ADC");
            strcpy(data.inputRegisters, "E");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x8C: {
            strcpy(data.mnemonic, "ADC");
            strcpy(data.inputRegisters, "H");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x8D: {
            strcpy(data.mnemonic, "ADC");
            strcpy(data.inputRegisters, "L");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x8E: {
            strcpy(data.mnemonic, "ADC");
            strcpy(data.inputRegisters, "M");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x8F: {
            strcpy(data.mnemonic, "ADC");
            strcpy(data.inputRegisters, "A");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x90: {
            strcpy(data.mnemonic, "SUB");
            strcpy(data.inputRegisters, "B");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x91: {
            strcpy(data.mnemonic, "SUB");
            strcpy(data.inputRegisters, "C");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x92: {
            strcpy(data.mnemonic, "SUB");
            strcpy(data.inputRegisters, "D");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x93: {
            strcpy(data.mnemonic, "SUB");
            strcpy(data.inputRegisters, "E");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x94: {
            strcpy(data.mnemonic, "SUB");
            strcpy(data.inputRegisters, "H");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x95: {
            strcpy(data.mnemonic, "SUB");
            strcpy(data.inputRegisters, "L");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x96: {
            strcpy(data.mnemonic, "SUB");
            strcpy(data.inputRegisters, "M");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x97: {
            strcpy(data.mnemonic, "SUB");
            strcpy(data.inputRegisters, "A");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x98: {
            strcpy(data.mnemonic, "SBB");
            strcpy(data.inputRegisters, "B");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x99: {
            strcpy(data.mnemonic, "SBB");
            strcpy(data.inputRegisters, "C");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x9A: {
            strcpy(data.mnemonic, "SBB");
            strcpy(data.inputRegisters, "D");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x9B: {
            strcpy(data.mnemonic, "SBB");
            strcpy(data.inputRegisters, "E");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x9C: {
            strcpy(data.mnemonic, "SBB");
            strcpy(data.inputRegisters, "H");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x9D: {
            strcpy(data.mnemonic, "SBB");
            strcpy(data.inputRegisters, "L");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x9E: {
            strcpy(data.mnemonic, "SBB");
            strcpy(data.inputRegisters, "M");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0x9F: {
            strcpy(data.mnemonic, "SBB");
            strcpy(data.inputRegisters, "A");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xA0: {
            strcpy(data.mnemonic, "ANA");
            strcpy(data.inputRegisters, "B");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xA1: {
            strcpy(data.mnemonic, "ANA");
            strcpy(data.inputRegisters, "C");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xA2: {
            strcpy(data.mnemonic, "ANA");
            strcpy(data.inputRegisters, "D");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xA3: {
            strcpy(data.mnemonic, "ANA");
            strcpy(data.inputRegisters, "E");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xA4: {
            strcpy(data.mnemonic, "ANA");
            strcpy(data.inputRegisters, "H");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xA5: {
            strcpy(data.mnemonic, "ANA");
            strcpy(data.inputRegisters, "L");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xA6: {
            strcpy(data.mnemonic, "ANA");
            strcpy(data.inputRegisters, "M");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xA7: {
            strcpy(data.mnemonic, "ANA");
            strcpy(data.inputRegisters, "A");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xA8: {
            strcpy(data.mnemonic, "XRA");
            strcpy(data.inputRegisters, "B");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xA9: {
            strcpy(data.mnemonic, "XRA");
            strcpy(data.inputRegisters, "C");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xAA: {
            strcpy(data.mnemonic, "XRA");
            strcpy(data.inputRegisters, "D");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xAB: {
            strcpy(data.mnemonic, "XRA");
            strcpy(data.inputRegisters, "E");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xAC: {
            strcpy(data.mnemonic, "XRA");
            strcpy(data.inputRegisters, "H");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xAD: {
            strcpy(data.mnemonic, "XRA");
            strcpy(data.inputRegisters, "L");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xAE: {
            strcpy(data.mnemonic, "XRA");
            strcpy(data.inputRegisters, "M");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xAF: {
            strcpy(data.mnemonic, "XRA");
            strcpy(data.inputRegisters, "A");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xB0: {
            strcpy(data.mnemonic, "ORA");
            strcpy(data.inputRegisters, "B");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xB1: {
            strcpy(data.mnemonic, "ORA");
            strcpy(data.inputRegisters, "C");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xB2: {
            strcpy(data.mnemonic, "ORA");
            strcpy(data.inputRegisters, "D");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xB3: {
            strcpy(data.mnemonic, "ORA");
            strcpy(data.inputRegisters, "E");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xB4: {
            strcpy(data.mnemonic, "ORA");
            strcpy(data.inputRegisters, "H");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xB5: {
            strcpy(data.mnemonic, "ORA");
            strcpy(data.inputRegisters, "L");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xB6: {
            strcpy(data.mnemonic, "ORA");
            strcpy(data.inputRegisters, "M");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xB7: {
            strcpy(data.mnemonic, "ORA");
            strcpy(data.inputRegisters, "A");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xB8: {
            strcpy(data.mnemonic, "CMP");
            strcpy(data.inputRegisters, "B");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xB9: {
            strcpy(data.mnemonic, "CMP");
            strcpy(data.inputRegisters, "C");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xBA: {
            strcpy(data.mnemonic, "CMP");
            strcpy(data.inputRegisters, "D");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xBB: {
            strcpy(data.mnemonic, "CMP");
            strcpy(data.inputRegisters, "E");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xBC: {
            strcpy(data.mnemonic, "CMP");
            strcpy(data.inputRegisters, "H");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xBD: {
            strcpy(data.mnemonic, "CMP");
            strcpy(data.inputRegisters, "L");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xBE: {
            strcpy(data.mnemonic, "CMP");
            strcpy(data.inputRegisters, "M");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xBF: {
            strcpy(data.mnemonic, "CMP");
            strcpy(data.inputRegisters, "A");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xC0: {
            strcpy(data.mnemonic, "RNZ");
            
            break;
        }
        case 0xC1: {
            strcpy(data.mnemonic, "POP");
            strcpy(data.inputRegisters, "B");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xC2: {
            strcpy(data.mnemonic, "JNZ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xC3: {
            strcpy(data.mnemonic, "JMP");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xC4: {
            strcpy(data.mnemonic, "CNZ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xC5: {
            strcpy(data.mnemonic, "PUSH");
            strcpy(data.inputRegisters, "B");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xC6: {
            strcpy(data.mnemonic, "ADI");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 0;
            data.num_inputValues = 1;
            break;
        }
        case 0xC7: {
            strcpy(data.mnemonic, "RST");
            strcpy(data.inputRegisters, "0");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xC8: {
            strcpy(data.mnemonic, "RZ");
            
            break;
        }
        case 0xC9: {
            strcpy(data.mnemonic, "RET");
            
            break;
        }
        case 0xCA: {
            strcpy(data.mnemonic, "JZ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xCB: {
            strcpy(data.mnemonic, "JMP");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xCC: {
            strcpy(data.mnemonic, "CZ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xCD: {
            strcpy(data.mnemonic, "CALL");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xCE: {
            strcpy(data.mnemonic, "ACI");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputValues = 1;
            break;
        }
        case 0xCF: {
            strcpy(data.mnemonic, "RST");
            strcpy(data.inputRegisters, "1");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xD0: {
            strcpy(data.mnemonic, "RNC");
            
            break;
        }
        case 0xD1: {
            strcpy(data.mnemonic, "POP");
            strcpy(data.inputRegisters, "D");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xD2: {
            strcpy(data.mnemonic, "JNC");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xD3: {
            strcpy(data.mnemonic, "OUT");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputValues = 1;
            break;
        }
        case 0xD4: {
            strcpy(data.mnemonic, "CNC");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xD5: {
            strcpy(data.mnemonic, "PUSH");
            strcpy(data.inputRegisters, "D");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xD6: {
            strcpy(data.mnemonic, "SUI");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 0;
            data.num_inputValues = 1;
            break;
        }
        case 0xD7: {
            strcpy(data.mnemonic, "RST");
            strcpy(data.inputRegisters, "2");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xD8: {
            strcpy(data.mnemonic, "RC");
            
            break;
        }
        case 0xD9: {
            strcpy(data.mnemonic, "RET");
            
            break;
        }
        case 0xDA: {
            strcpy(data.mnemonic, "JC");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xDB: {
            strcpy(data.mnemonic, "IN");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputValues = 1;
            break;
        }
        case 0xDC: {
            strcpy(data.mnemonic, "CC");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xDD: {
            strcpy(data.mnemonic, "CALL");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xDE: {
            strcpy(data.mnemonic, "SBI");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputValues = 1;
            break;
        }
        case 0xDF: {
            strcpy(data.mnemonic, "RST");
            strcpy(data.inputRegisters, "3");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xE0: {
            strcpy(data.mnemonic, "RPO");
            
            break;
        }
        case 0xE1: {
            strcpy(data.mnemonic, "POP");
            strcpy(data.inputRegisters, "H");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xE2: {
            strcpy(data.mnemonic, "JPO");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xE3: {
            strcpy(data.mnemonic, "XTHL");
            
            break;
        }
        case 0xE4: {
            strcpy(data.mnemonic, "CPO");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xE5: {
            strcpy(data.mnemonic, "PUSH");
            strcpy(data.inputRegisters, "H");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xE6: {
            strcpy(data.mnemonic, "ANI");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 0;
            data.num_inputValues = 1;
            break;
        }
        case 0xE7: {
            strcpy(data.mnemonic, "RST");
            strcpy(data.inputRegisters, "4");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xE8: {
            strcpy(data.mnemonic, "RPE");
            
            break;
        }
        case 0xE9: {
            strcpy(data.mnemonic, "PCHL");
            
            break;
        }
        case 0xEA: {
            strcpy(data.mnemonic, "JPE");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xEB: {
            strcpy(data.mnemonic, "XCHG");
            
            break;
        }
        case 0xEC: {
            strcpy(data.mnemonic, "CPE");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xED: {
            strcpy(data.mnemonic, "CALL");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xEE: {
            strcpy(data.mnemonic, "XRI");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputValues = 1;
            break;
        }
        case 0xEF: {
            strcpy(data.mnemonic, "RST");
            strcpy(data.inputRegisters, "5");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xF0: {
            strcpy(data.mnemonic, "RP");
            
            break;
        }
        case 0xF1: {
            strcpy(data.mnemonic, "POP");
            strcpy(data.inputRegisters, "PSW");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xF2: {
            strcpy(data.mnemonic, "JP");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xF3: {
            strcpy(data.mnemonic, "DI");
            
            break;
        }
        case 0xF4: {
            strcpy(data.mnemonic, "CP");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xF5: {
            strcpy(data.mnemonic, "PUSH");
            strcpy(data.inputRegisters, "PSW");
            
            data.num_inputRegisters = 1 ;
            break;
        }
        case 0xF6: {
            strcpy(data.mnemonic, "ORI");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 0;
            data.num_inputValues = 1;
            break;
        }
        case 0xF7: {
            strcpy(data.mnemonic, "RST");
            strcpy(data.inputRegisters, "6");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xF8: {
            strcpy(data.mnemonic, "RM");
            
            break;
        }
        case 0xF9: {
            strcpy(data.mnemonic, "SPHL");
            
            break;
        }
        case 0xFA: {
            strcpy(data.mnemonic, "JM");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xFB: {
            strcpy(data.mnemonic, "EI");
            
            break;
        }
        case 0xFC: {
            strcpy(data.mnemonic, "CM");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xFD: {
            strcpy(data.mnemonic, "CALL");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xFE: {
            strcpy(data.mnemonic, "CPI");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputValues = 1;
            break;
        }
        case 0xFF: {
            strcpy(data.mnemonic, "RST");
            strcpy(data.inputRegisters, "7");
            
            data.num_inputRegisters = 1;
            break;
        }
    }
    
    return data ;
}

#define AVERAGE_CHARS_PER_PROGRAM_BYTE  15

#include "vstring.h"

char* disassemble_program(bytestream_t program) {
    vstring_t* result = vstring_new(AVERAGE_CHARS_PER_PROGRAM_BYTE * program.size * sizeof(char));

    size_t programPointer = 0;
    while (programPointer < program.size) {
        instruction8080_t instruction = disassemble_instruction(program.data, programPointer);
        char* line = instruction_toString(instruction);

        vstring_concat(result, line);
        programPointer += instruction.instructionLength;
    }
}