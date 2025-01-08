#include <string.h>
#include "disassembler8080.h"

int abs(int x) {
    if(x>=0)
        return x;
    else
        return -x;
}

void printline(disassembler8080_instruction_t dt, FILE* ofp) {
    int i;
    
    fprintf(ofp, "%04X:\t", dt.position);
    fprintf(ofp, "%02X", (unsigned int)dt.opcode);
    
    for(i=0;i<dt.num_inputValues;i++) {
        fputc(' ', ofp);
        fprintf(ofp, "%02X", abs(dt.inputValues[i]));
    }
    fputc('\t', ofp);
    if(dt.num_inputValues<2)
        fputc('\t', ofp);
    fprintf(ofp, "%s\t", dt.mnemonic);
    
    if(dt.num_inputRegisters > 0) {
        fprintf(ofp, "%s", dt.inputRegisters);
        if(dt.num_inputValues > 0) fputc(',',ofp);
            else fprintf(ofp, "\t");
    }
    if(dt.num_inputValues > 1){
        #if DISASSEMBLER8080_LITTLE_ENDIAN
        if(dt.inputValues[dt.num_inputValues-1] > 0) fputc('#',ofp);        //
            else fputc('$',ofp);                                            //
        fprintf(ofp, "%02X", abs(dt.inputValues[dt.num_inputValues-1]));    //
        fprintf(ofp, "%02X\t", abs(dt.inputValues[dt.num_inputValues-2]));  //
        #else
        if(dt.inputValues[dt.num_inputValues-1] > 0) fputc('#',ofp);        // assunzione implicita:
            else fputc('$',ofp);                                            // i primi due byte successivi all'opcode
        fprintf(ofp, "%02X", abs(dt.inputValues[dt.num_inputValues-2]));    // compongono un unico parametro da 16 bit
        fprintf(ofp, "%02X\t", abs(dt.inputValues[dt.num_inputValues-1]));  // DA RIGUARDARE perché al momento non
        #endif                                                              // supporta altri valori di MAX_INSTRUCTION_PARAMETERS
    }            
    else if(dt.num_inputValues > 0){
        if(dt.inputValues[dt.num_inputValues-1] >= 0) fputc('#',ofp);
            else fputc('$',ofp);
        fprintf(ofp, "%02X\t", abs(dt.inputValues[dt.num_inputValues-1]));    
    }
    
    fprintf(ofp, "\n");
    return;
}


// raccoglie tutte le info sulla istruzione fornita e le restituisce senza stamparle
disassembler8080_instruction_t disassemble_instruction( uint8_t* mem , unsigned int addr ) {
    disassembler8080_instruction_t dt;
    
    dt.opcode = (unsigned char) mem[addr];
    
    dt.position = addr;
    dt.num_inputRegisters = 0;     //
    dt.num_inputValues = 0;        //
    dt.instructionLength = 1;      // default values
    
    switch (dt.opcode) {
        case 0x00: {
            strcpy(dt.mnemonic,"NOP");
            dt.instructionLength = 1;
            break;
        }
        case 0x01: {
            strcpy(dt.mnemonic, "LXI");
            strcpy(dt.inputRegisters, "B");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            dt.inputValues[1] = + (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputRegisters = 1;
            dt.num_inputValues = 2;
            break;
        }
        case 0x02: {
            strcpy(dt.mnemonic, "STAX");
            strcpy(dt.inputRegisters, "B");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x03: {
            strcpy(dt.mnemonic, "INX");
            strcpy(dt.inputRegisters, "B");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x04: {
            strcpy(dt.mnemonic, "INR");
            strcpy(dt.inputRegisters, "B");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x05: {
            strcpy(dt.mnemonic, "DCR");
            strcpy(dt.inputRegisters, "B");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x06: {
            strcpy(dt.mnemonic, "MVI");
            strcpy(dt.inputRegisters, "B");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            
            dt.instructionLength = 2;
            dt.num_inputRegisters = 1;
            dt.num_inputValues = 1;
            break;
        }
        case 0x07: {
            strcpy(dt.mnemonic,"RLC");
            dt.instructionLength = 1;
            break;
        }
        case 0x08: {
            strcpy(dt.mnemonic,"NOP");
            dt.instructionLength = 1;
            break;
        }
        case 0x09: {
            strcpy(dt.mnemonic, "DAD");
            strcpy(dt.inputRegisters, "B");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x0A: {
            strcpy(dt.mnemonic, "LDAX");
            strcpy(dt.inputRegisters, "B");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x0B: {
            strcpy(dt.mnemonic, "DCX");
            strcpy(dt.inputRegisters, "B");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x0C: {
            strcpy(dt.mnemonic, "INR");
            strcpy(dt.inputRegisters, "C");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x0D: {
            strcpy(dt.mnemonic, "DCR");
            strcpy(dt.inputRegisters, "C");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x0E: {
            strcpy(dt.mnemonic, "MVI");
            strcpy(dt.inputRegisters, "C");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            
            dt.instructionLength = 2;
            dt.num_inputRegisters = 1;
            dt.num_inputValues = 1;
            break;
        }
        case 0x0F: {
            strcpy(dt.mnemonic, "RRC");
            
            dt.instructionLength = 1;
            break;
        }
        case 0x10: {
            strcpy(dt.mnemonic, "NOP");
            
            dt.instructionLength = 1;
            break;
        }
        case 0x11: {
            strcpy(dt.mnemonic, "LXI");
            strcpy(dt.inputRegisters, "D");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            dt.inputValues[1] = + (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputRegisters = 1;
            dt.num_inputValues = 2;
            break;
        }
        case 0x12: {
            strcpy(dt.mnemonic, "STAX");
            strcpy(dt.inputRegisters, "D");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x13: {
            strcpy(dt.mnemonic, "INX");
            strcpy(dt.inputRegisters, "D");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x14: {
            strcpy(dt.mnemonic, "INR");
            strcpy(dt.inputRegisters, "D");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x15: {
            strcpy(dt.mnemonic, "DCR");
            strcpy(dt.inputRegisters, "D");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x16: {
            strcpy(dt.mnemonic, "MVI");
            strcpy(dt.inputRegisters, "D");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            
            dt.instructionLength = 2;
            dt.num_inputRegisters = 1;
            dt.num_inputValues = 1;
            break;
        }
        case 0x17: {
            strcpy(dt.mnemonic,"RAL");
            dt.instructionLength = 1;
            break;
        }
        case 0x18: {
            strcpy(dt.mnemonic,"NOP");
            dt.instructionLength = 1;
            break;
        }
        case 0x19: {
            strcpy(dt.mnemonic, "DAD");
            strcpy(dt.inputRegisters, "D");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x1A: {
            strcpy(dt.mnemonic, "LDAX");
            strcpy(dt.inputRegisters, "D");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x1B: {
            strcpy(dt.mnemonic, "DCX");
            strcpy(dt.inputRegisters, "E");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x1C: {
            strcpy(dt.mnemonic, "INR");
            strcpy(dt.inputRegisters, "E");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x1D: {
            strcpy(dt.mnemonic, "DCR");
            strcpy(dt.inputRegisters, "E");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x1E: {
            strcpy(dt.mnemonic, "MVI");
            strcpy(dt.inputRegisters, "E");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            
            dt.instructionLength = 2;
            dt.num_inputRegisters = 1;
            dt.num_inputValues = 1;
            break;
        }
        case 0x1F: {
            strcpy(dt.mnemonic, "RAR");
            
            dt.instructionLength = 1;
            break;
        }
        case 0x20: {
            strcpy(dt.mnemonic, "NOP");
            
            dt.instructionLength = 1;
            break;
        }
        case 0x21: {
            strcpy(dt.mnemonic, "LXI");
            strcpy(dt.inputRegisters, "H");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            dt.inputValues[1] = + (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputRegisters = 1;
            dt.num_inputValues = 2;
            break;
        }
        case 0x22: {
            strcpy(dt.mnemonic, "SHLD");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputRegisters = 0;
            dt.num_inputValues = 2;
            break;
        }
        case 0x23: {
            strcpy(dt.mnemonic, "INX");
            strcpy(dt.inputRegisters, "H");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x24: {
            strcpy(dt.mnemonic, "INR");
            strcpy(dt.inputRegisters, "H");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x25: {
            strcpy(dt.mnemonic, "DCR");
            strcpy(dt.inputRegisters, "H");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x26: {
            strcpy(dt.mnemonic, "MVI");
            strcpy(dt.inputRegisters, "H");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            
            dt.instructionLength = 2;
            dt.num_inputRegisters = 1;
            dt.num_inputValues = 1;
            break;
        }
        case 0x27: {
            strcpy(dt.mnemonic,"DAA");
            dt.instructionLength = 1;
            break;
        }
        case 0x28: {
            strcpy(dt.mnemonic,"NOP");
            dt.instructionLength = 1;
            break;
        }
        case 0x29: {
            strcpy(dt.mnemonic, "DAD");
            strcpy(dt.inputRegisters, "H");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x2A: {
            strcpy(dt.mnemonic, "LHLD");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+1];
            
            dt.instructionLength = 3;
            dt.num_inputRegisters = 0;
            dt.num_inputValues = 2;
            break;
        }
        case 0x2B: {
            strcpy(dt.mnemonic, "DCX");
            strcpy(dt.inputRegisters, "H");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x2C: {
            strcpy(dt.mnemonic, "INR");
            strcpy(dt.inputRegisters, "L");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x2D: {
            strcpy(dt.mnemonic, "DCR");
            strcpy(dt.inputRegisters, "L");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x2E: {
            strcpy(dt.mnemonic, "MVI");
            strcpy(dt.inputRegisters, "L");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            
            dt.instructionLength = 2;
            dt.num_inputRegisters = 1;
            dt.num_inputValues = 1;
            break;
        }
        case 0x2F: {
            strcpy(dt.mnemonic, "CMA");
            
            dt.instructionLength = 1;
            break;
        }
        case 0x30: {
            strcpy(dt.mnemonic, "NOP");
            
            dt.instructionLength = 1;
            break;
        }
        case 0x31: {
            strcpy(dt.mnemonic, "LXI");
            strcpy(dt.inputRegisters, "SP");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            dt.inputValues[1] = + (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputRegisters = 1;
            dt.num_inputValues = 2;
            break;
        }
        case 0x32: {
            strcpy(dt.mnemonic, "STA");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputRegisters = 0;
            dt.num_inputValues = 2;
            break;
        }
        case 0x33: {
            strcpy(dt.mnemonic, "INX");
            strcpy(dt.inputRegisters, "SP");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x34: {
            strcpy(dt.mnemonic, "INR");
            strcpy(dt.inputRegisters, "M");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x35: {
            strcpy(dt.mnemonic, "DCR");
            strcpy(dt.inputRegisters, "M");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x36: {
            strcpy(dt.mnemonic, "MVI");
            strcpy(dt.inputRegisters, "M");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            
            dt.instructionLength = 2;
            dt.num_inputRegisters = 1;
            dt.num_inputValues = 1;
            break;
        }
        case 0x37: {
            strcpy(dt.mnemonic,"STC");
            
            dt.instructionLength = 1;
            break;
        }
        case 0x38: {
            strcpy(dt.mnemonic,"NOP");
            
            dt.instructionLength = 1;
            break;
        }
        case 0x39: {
            strcpy(dt.mnemonic, "DAD");
            strcpy(dt.inputRegisters, "SP");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x3A: {
            strcpy(dt.mnemonic, "LDA");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputRegisters = 0;
            dt.num_inputValues = 2;
            break;
        }
        case 0x3B: {
            strcpy(dt.mnemonic, "DCX");
            strcpy(dt.inputRegisters, "SP");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x3C: {
            strcpy(dt.mnemonic, "INR");
            strcpy(dt.inputRegisters, "A");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x3D: {
            strcpy(dt.mnemonic, "DCR");
            strcpy(dt.inputRegisters, "A");
            
            dt.instructionLength = 1;
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x3E: {
            strcpy(dt.mnemonic, "MVI");
            strcpy(dt.inputRegisters, "A");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            
            dt.instructionLength = 2;
            dt.num_inputRegisters = 1;
            dt.num_inputValues = 1;
            break;
        }
        case 0x3F: {
            strcpy(dt.mnemonic, "CMC");
            
            dt.instructionLength = 1;
            break;
        }
        case 0x40: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "B,B");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x41: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "B,C");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x42: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "B,D");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x43: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "B,E");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x44: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "B,H");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x45: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "B,L");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x46: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "B,M");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x47: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "B,A");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x48: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "C,B");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x49: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "C,C");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x4A: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "C,D");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x4B: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "C,E");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x4C: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "C,H");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x4D: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "C,L");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x4E: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "C,M");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x4F: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "C,A");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x50: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "D,B");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x51: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "D,C");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x52: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "D,D");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x53: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "D,E");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x54: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "D,H");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x55: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "D,L");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x56: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "D,M");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x57: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "D,A");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x58: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "E,B");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x59: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "E,C");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x5A: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "E,D");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x5B: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "E,E");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x5C: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "E,H");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x5D: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "E,L");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x5E: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "E,M");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x5F: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "E,A");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x60: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "H,B");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x61: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "H,C");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x62: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "H,D");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x63: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "H,E");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x64: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "H,H");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x65: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "H,L");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x66: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "H,M");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x67: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "H,A");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x68: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "L,B");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x69: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "L,C");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x6A: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "L,D");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x6B: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "L,E");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x6C: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "L,H");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x6D: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "L,L");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x6E: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "L,M");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x6F: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "L,A");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x70: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "M,B");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x71: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "M,C");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x72: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "M,D");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x73: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "M,E");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x74: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "M,H");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x75: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "M,L");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x76: {
            strcpy(dt.mnemonic, "HLT");
            
            dt.instructionLength = 1;
            break;
        }
        case 0x77: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "M,A");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x78: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "A,B");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x79: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "A,C");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x7A: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "A,D");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x7B: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "A,E");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x7C: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "A,H");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x7D: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "A,L");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x7E: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "A,M");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x7F: {
            strcpy(dt.mnemonic, "MOV");
            strcpy(dt.inputRegisters, "A,A");
            
            dt.num_inputRegisters = 2;
            break;
        }
        case 0x80: {
            strcpy(dt.mnemonic, "ADD");
            strcpy(dt.inputRegisters, "B");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x81: {
            strcpy(dt.mnemonic, "ADD");
            strcpy(dt.inputRegisters, "C");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x82: {
            strcpy(dt.mnemonic, "ADD");
            strcpy(dt.inputRegisters, "D");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x83: {
            strcpy(dt.mnemonic, "ADD");
            strcpy(dt.inputRegisters, "E");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x84: {
            strcpy(dt.mnemonic, "ADD");
            strcpy(dt.inputRegisters, "H");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x85: {
            strcpy(dt.mnemonic, "ADD");
            strcpy(dt.inputRegisters, "L");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x86: {
            strcpy(dt.mnemonic, "ADD");
            strcpy(dt.inputRegisters, "M");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x87: {
            strcpy(dt.mnemonic, "ADD");
            strcpy(dt.inputRegisters, "A");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x88: {
            strcpy(dt.mnemonic, "ADC");
            strcpy(dt.inputRegisters, "B");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x89: {
            strcpy(dt.mnemonic, "ADC");
            strcpy(dt.inputRegisters, "C");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x8A: {
            strcpy(dt.mnemonic, "ADC");
            strcpy(dt.inputRegisters, "D");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x8B: {
            strcpy(dt.mnemonic, "ADC");
            strcpy(dt.inputRegisters, "E");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x8C: {
            strcpy(dt.mnemonic, "ADC");
            strcpy(dt.inputRegisters, "H");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x8D: {
            strcpy(dt.mnemonic, "ADC");
            strcpy(dt.inputRegisters, "L");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x8E: {
            strcpy(dt.mnemonic, "ADC");
            strcpy(dt.inputRegisters, "M");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x8F: {
            strcpy(dt.mnemonic, "ADC");
            strcpy(dt.inputRegisters, "A");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x90: {
            strcpy(dt.mnemonic, "SUB");
            strcpy(dt.inputRegisters, "B");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x91: {
            strcpy(dt.mnemonic, "SUB");
            strcpy(dt.inputRegisters, "C");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x92: {
            strcpy(dt.mnemonic, "SUB");
            strcpy(dt.inputRegisters, "D");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x93: {
            strcpy(dt.mnemonic, "SUB");
            strcpy(dt.inputRegisters, "E");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x94: {
            strcpy(dt.mnemonic, "SUB");
            strcpy(dt.inputRegisters, "H");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x95: {
            strcpy(dt.mnemonic, "SUB");
            strcpy(dt.inputRegisters, "L");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x96: {
            strcpy(dt.mnemonic, "SUB");
            strcpy(dt.inputRegisters, "M");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x97: {
            strcpy(dt.mnemonic, "SUB");
            strcpy(dt.inputRegisters, "A");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x98: {
            strcpy(dt.mnemonic, "SBB");
            strcpy(dt.inputRegisters, "B");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x99: {
            strcpy(dt.mnemonic, "SBB");
            strcpy(dt.inputRegisters, "C");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x9A: {
            strcpy(dt.mnemonic, "SBB");
            strcpy(dt.inputRegisters, "D");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x9B: {
            strcpy(dt.mnemonic, "SBB");
            strcpy(dt.inputRegisters, "E");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x9C: {
            strcpy(dt.mnemonic, "SBB");
            strcpy(dt.inputRegisters, "H");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x9D: {
            strcpy(dt.mnemonic, "SBB");
            strcpy(dt.inputRegisters, "L");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x9E: {
            strcpy(dt.mnemonic, "SBB");
            strcpy(dt.inputRegisters, "M");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0x9F: {
            strcpy(dt.mnemonic, "SBB");
            strcpy(dt.inputRegisters, "A");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xA0: {
            strcpy(dt.mnemonic, "ANA");
            strcpy(dt.inputRegisters, "B");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xA1: {
            strcpy(dt.mnemonic, "ANA");
            strcpy(dt.inputRegisters, "C");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xA2: {
            strcpy(dt.mnemonic, "ANA");
            strcpy(dt.inputRegisters, "D");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xA3: {
            strcpy(dt.mnemonic, "ANA");
            strcpy(dt.inputRegisters, "E");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xA4: {
            strcpy(dt.mnemonic, "ANA");
            strcpy(dt.inputRegisters, "H");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xA5: {
            strcpy(dt.mnemonic, "ANA");
            strcpy(dt.inputRegisters, "L");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xA6: {
            strcpy(dt.mnemonic, "ANA");
            strcpy(dt.inputRegisters, "M");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xA7: {
            strcpy(dt.mnemonic, "ANA");
            strcpy(dt.inputRegisters, "A");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xA8: {
            strcpy(dt.mnemonic, "XRA");
            strcpy(dt.inputRegisters, "B");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xA9: {
            strcpy(dt.mnemonic, "XRA");
            strcpy(dt.inputRegisters, "C");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xAA: {
            strcpy(dt.mnemonic, "XRA");
            strcpy(dt.inputRegisters, "D");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xAB: {
            strcpy(dt.mnemonic, "XRA");
            strcpy(dt.inputRegisters, "E");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xAC: {
            strcpy(dt.mnemonic, "XRA");
            strcpy(dt.inputRegisters, "H");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xAD: {
            strcpy(dt.mnemonic, "XRA");
            strcpy(dt.inputRegisters, "L");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xAE: {
            strcpy(dt.mnemonic, "XRA");
            strcpy(dt.inputRegisters, "M");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xAF: {
            strcpy(dt.mnemonic, "XRA");
            strcpy(dt.inputRegisters, "A");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xB0: {
            strcpy(dt.mnemonic, "ORA");
            strcpy(dt.inputRegisters, "B");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xB1: {
            strcpy(dt.mnemonic, "ORA");
            strcpy(dt.inputRegisters, "C");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xB2: {
            strcpy(dt.mnemonic, "ORA");
            strcpy(dt.inputRegisters, "D");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xB3: {
            strcpy(dt.mnemonic, "ORA");
            strcpy(dt.inputRegisters, "E");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xB4: {
            strcpy(dt.mnemonic, "ORA");
            strcpy(dt.inputRegisters, "H");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xB5: {
            strcpy(dt.mnemonic, "ORA");
            strcpy(dt.inputRegisters, "L");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xB6: {
            strcpy(dt.mnemonic, "ORA");
            strcpy(dt.inputRegisters, "M");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xB7: {
            strcpy(dt.mnemonic, "ORA");
            strcpy(dt.inputRegisters, "A");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xB8: {
            strcpy(dt.mnemonic, "CMP");
            strcpy(dt.inputRegisters, "B");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xB9: {
            strcpy(dt.mnemonic, "CMP");
            strcpy(dt.inputRegisters, "C");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xBA: {
            strcpy(dt.mnemonic, "CMP");
            strcpy(dt.inputRegisters, "D");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xBB: {
            strcpy(dt.mnemonic, "CMP");
            strcpy(dt.inputRegisters, "E");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xBC: {
            strcpy(dt.mnemonic, "CMP");
            strcpy(dt.inputRegisters, "H");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xBD: {
            strcpy(dt.mnemonic, "CMP");
            strcpy(dt.inputRegisters, "L");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xBE: {
            strcpy(dt.mnemonic, "CMP");
            strcpy(dt.inputRegisters, "M");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xBF: {
            strcpy(dt.mnemonic, "CMP");
            strcpy(dt.inputRegisters, "A");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xC0: {
            strcpy(dt.mnemonic, "RNZ");
            
            break;
        }
        case 0xC1: {
            strcpy(dt.mnemonic, "POP");
            strcpy(dt.inputRegisters, "B");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xC2: {
            strcpy(dt.mnemonic, "JNZ");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xC3: {
            strcpy(dt.mnemonic, "JMP");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xC4: {
            strcpy(dt.mnemonic, "CNZ");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xC5: {
            strcpy(dt.mnemonic, "PUSH");
            strcpy(dt.inputRegisters, "B");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xC6: {
            strcpy(dt.mnemonic, "ADI");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            
            dt.instructionLength = 2;
            dt.num_inputRegisters = 0;
            dt.num_inputValues = 1;
            break;
        }
        case 0xC7: {
            strcpy(dt.mnemonic, "RST");
            strcpy(dt.inputRegisters, "0");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xC8: {
            strcpy(dt.mnemonic, "RZ");
            
            break;
        }
        case 0xC9: {
            strcpy(dt.mnemonic, "RET");
            
            break;
        }
        case 0xCA: {
            strcpy(dt.mnemonic, "JZ");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xCB: {
            strcpy(dt.mnemonic, "JMP");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xCC: {
            strcpy(dt.mnemonic, "CZ");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xCD: {
            strcpy(dt.mnemonic, "CALL");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xCE: {
            strcpy(dt.mnemonic, "ACI");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            
            dt.instructionLength = 2;
            dt.num_inputValues = 1;
            break;
        }
        case 0xCF: {
            strcpy(dt.mnemonic, "RST");
            strcpy(dt.inputRegisters, "1");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xD0: {
            strcpy(dt.mnemonic, "RNC");
            
            break;
        }
        case 0xD1: {
            strcpy(dt.mnemonic, "POP");
            strcpy(dt.inputRegisters, "D");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xD2: {
            strcpy(dt.mnemonic, "JNC");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xD3: {
            strcpy(dt.mnemonic, "OUT");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            
            dt.instructionLength = 2;
            dt.num_inputValues = 1;
            break;
        }
        case 0xD4: {
            strcpy(dt.mnemonic, "CNC");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xD5: {
            strcpy(dt.mnemonic, "PUSH");
            strcpy(dt.inputRegisters, "D");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xD6: {
            strcpy(dt.mnemonic, "SUI");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            
            dt.instructionLength = 2;
            dt.num_inputRegisters = 0;
            dt.num_inputValues = 1;
            break;
        }
        case 0xD7: {
            strcpy(dt.mnemonic, "RST");
            strcpy(dt.inputRegisters, "2");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xD8: {
            strcpy(dt.mnemonic, "RC");
            
            break;
        }
        case 0xD9: {
            strcpy(dt.mnemonic, "RET");
            
            break;
        }
        case 0xDA: {
            strcpy(dt.mnemonic, "JC");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xDB: {
            strcpy(dt.mnemonic, "IN");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            
            dt.instructionLength = 2;
            dt.num_inputValues = 1;
            break;
        }
        case 0xDC: {
            strcpy(dt.mnemonic, "CC");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xDD: {
            strcpy(dt.mnemonic, "CALL");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xDE: {
            strcpy(dt.mnemonic, "SBI");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            
            dt.instructionLength = 2;
            dt.num_inputValues = 1;
            break;
        }
        case 0xDF: {
            strcpy(dt.mnemonic, "RST");
            strcpy(dt.inputRegisters, "3");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xE0: {
            strcpy(dt.mnemonic, "RPO");
            
            break;
        }
        case 0xE1: {
            strcpy(dt.mnemonic, "POP");
            strcpy(dt.inputRegisters, "H");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xE2: {
            strcpy(dt.mnemonic, "JPO");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xE3: {
            strcpy(dt.mnemonic, "XTHL");
            
            break;
        }
        case 0xE4: {
            strcpy(dt.mnemonic, "CPO");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xE5: {
            strcpy(dt.mnemonic, "PUSH");
            strcpy(dt.inputRegisters, "H");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xE6: {
            strcpy(dt.mnemonic, "ANI");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            
            dt.instructionLength = 2;
            dt.num_inputRegisters = 0;
            dt.num_inputValues = 1;
            break;
        }
        case 0xE7: {
            strcpy(dt.mnemonic, "RST");
            strcpy(dt.inputRegisters, "4");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xE8: {
            strcpy(dt.mnemonic, "RPE");
            
            break;
        }
        case 0xE9: {
            strcpy(dt.mnemonic, "PCHL");
            
            break;
        }
        case 0xEA: {
            strcpy(dt.mnemonic, "JPE");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xEB: {
            strcpy(dt.mnemonic, "XCHG");
            
            break;
        }
        case 0xEC: {
            strcpy(dt.mnemonic, "CPE");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xED: {
            strcpy(dt.mnemonic, "CALL");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xEE: {
            strcpy(dt.mnemonic, "XRI");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            
            dt.instructionLength = 2;
            dt.num_inputValues = 1;
            break;
        }
        case 0xEF: {
            strcpy(dt.mnemonic, "RST");
            strcpy(dt.inputRegisters, "5");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xF0: {
            strcpy(dt.mnemonic, "RP");
            
            break;
        }
        case 0xF1: {
            strcpy(dt.mnemonic, "POP");
            strcpy(dt.inputRegisters, "PSW");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xF2: {
            strcpy(dt.mnemonic, "JP");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xF3: {
            strcpy(dt.mnemonic, "DI");
            
            break;
        }
        case 0xF4: {
            strcpy(dt.mnemonic, "CP");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xF5: {
            strcpy(dt.mnemonic, "PUSH");
            strcpy(dt.inputRegisters, "PSW");
            
            dt.num_inputRegisters = 1 ;
            break;
        }
        case 0xF6: {
            strcpy(dt.mnemonic, "ORI");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            
            dt.instructionLength = 2;
            dt.num_inputRegisters = 0;
            dt.num_inputValues = 1;
            break;
        }
        case 0xF7: {
            strcpy(dt.mnemonic, "RST");
            strcpy(dt.inputRegisters, "6");
            
            dt.num_inputRegisters = 1;
            break;
        }
        case 0xF8: {
            strcpy(dt.mnemonic, "RM");
            
            break;
        }
        case 0xF9: {
            strcpy(dt.mnemonic, "SPHL");
            
            break;
        }
        case 0xFA: {
            strcpy(dt.mnemonic, "JM");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xFB: {
            strcpy(dt.mnemonic, "EI");
            
            break;
        }
        case 0xFC: {
            strcpy(dt.mnemonic, "CM");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xFD: {
            strcpy(dt.mnemonic, "CALL");
            
            dt.inputValues[0] = - (int) mem[addr+1];
            dt.inputValues[1] = - (int) mem[addr+2];
            
            dt.instructionLength = 3;
            dt.num_inputValues = 2;
            break;
        }
        case 0xFE: {
            strcpy(dt.mnemonic, "CPI");
            
            dt.inputValues[0] = + (int) mem[addr+1];
            
            dt.instructionLength = 2;
            dt.num_inputValues = 1;
            break;
        }
        case 0xFF: {
            strcpy(dt.mnemonic, "RST");
            strcpy(dt.inputRegisters, "7");
            
            dt.num_inputRegisters = 1;
            break;
        }
    }
    
    return dt ;
}