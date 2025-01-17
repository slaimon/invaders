#include <string.h>
#include <stdlib.h>
#include "../include/i8080_disassembler.h"

void disassemble_program(const bytestream_t program, FILE* ofp) {
    char result[I8080_LINE_LENGTH * program.size];

    size_t programPointer = 0;
    size_t stringPointer = 0;
    while (programPointer < program.size) {
        i8080_instruction_t instruction = disassemble_instruction(program.data, programPointer);
        i8080_line_t line = i8080_instruction_toString(instruction, true);

        stringPointer += sprintf(&result[stringPointer], "%s\n", line.string);
        programPointer += instruction.instructionLength;
    }

    fprintf(ofp, result);
}


/* ---------------- INSTRUCTION_TOSTRING ---------------- */

// the mnemonics are placed at this column
#define MNEMONIC_COLUMN_NUMBER  22
// the comments are placed at this column
#define COMMENT_COLUMN_NUMBER   40

#define INSERT_WHITESPACE(n)        \
    for(int i = 0; i < n; ++i)      \
        result[pointer++] = ' ';

#define PAD_TO(n)                 \
    while(pointer < (n))          \
        result[pointer++] = ' ';

i8080_line_t i8080_instruction_toString(i8080_instruction_t instruction, bool generateComments) {
    i8080_line_t line;
    char* result = line.string;
    memset(result, '\0', I8080_LINE_LENGTH);
    bool hasComment = (instruction.comment[0] != '\0');
    size_t pointer = 0;

    // address gutter on the left
    pointer += sprintf(&result[pointer], "%04X:  ", instruction.position);

    // binary representation of the instruction (or padding if needed)
    pointer += sprintf(&result[pointer], "%02X", instruction.opcode);
    for(int i = 0; i < instruction.num_inputValues; ++i) {
        pointer += sprintf(&result[pointer], " %02X", abs(instruction.inputValues[i]));
    }

    // the instruction's mnemonic
    PAD_TO(MNEMONIC_COLUMN_NUMBER)
    pointer += sprintf(&result[pointer], instruction.mnemonic);

    // register arguments, if any
    INSERT_WHITESPACE(2)
    if (instruction.num_inputRegisters > 0) {
        if (instruction.num_inputValues == 0)
            pointer += sprintf(&result[pointer], instruction.inputRegisters);
        else
            pointer += sprintf(&result[pointer], "%s, ", instruction.inputRegisters);
    }
    else if (instruction.num_inputValues == 0 && hasComment)
        INSERT_WHITESPACE(4)
    
    // numerical arguments, if any
    // immediate arguments are prefixed with '#'
    // while addresses are prefixed with '$'
    if (instruction.num_inputValues == 2) {
        char prefix = instruction.immediate ? '#' : '$';
        #if I8080_LITTLE_ENDIAN
            uint8_t first = abs(instruction.inputValues[1]);
            uint8_t second = abs(instruction.inputValues[0]);
        #else
            uint8_t first = abs(instruction.inputValues[0]);
            uint8_t second = abs(instruction.inputValues[1]);
        #endif
        pointer += sprintf(&result[pointer], "%c%02X%02X", prefix, first, second);
    }
    else if (instruction.num_inputValues == 1) {
        char prefix = instruction.immediate ? '#' : '$';
        pointer += sprintf(&result[pointer], "%c%02X", prefix, abs(instruction.inputValues[0]));
        INSERT_WHITESPACE(2)
    }

    // comment, if any
    if (hasComment && generateComments) {
        PAD_TO(COMMENT_COLUMN_NUMBER)
        pointer += sprintf(&result[pointer], "; %s", instruction.comment);
    }

    return line;
}


/* ---------------- COMMENT GENERATOR MACROS ---------------- */

#define COMMENT_LXI(x)      sprintf(data.comment, "Load immediate value to %s", x);
#define COMMENT_STAX(x)     sprintf(data.comment, "Store A to $%s", x);
#define COMMENT_INX(x)      sprintf(data.comment, "Increment %s", x);
#define COMMENT_INR(x)      COMMENT_INX(x)
#define COMMENT_DCX(x)      sprintf(data.comment, "Decrement %s", x);
#define COMMENT_DCR(x)      COMMENT_DCX(x)
#define COMMENT_MVI(x)      sprintf(data.comment, "Move immediate value to %s", x);
#define COMMENT_DAD(x)      sprintf(data.comment, "Add %s to HL", x);
#define COMMENT_RLC         strcpy (data.comment, "Rotate A left, set Carry flag");
#define COMMENT_RRC         strcpy (data.comment, "Rotate A right, set Carry flag");
#define COMMENT_RAL         strcpy (data.comment, "Rotate A left");
#define COMMENT_RAR         strcpy (data.comment, "Rotate A right");
#define COMMENT_LDAX(x)     sprintf(data.comment, "Load A from $%s (indirect)", x);
#define COMMENT_SHLD        strcpy (data.comment, "Store HL to address");
#define COMMENT_LHLD        strcpy (data.comment, "Load HL from address");
#define COMMENT_DAA         strcpy (data.comment, "Convert A to binary-coded decimal");
#define COMMENT_CMA         strcpy (data.comment, "Apply one's complement to A");
#define COMMENT_STA         strcpy (data.comment, "Store A to address");
#define COMMENT_LDA         strcpy (data.comment, "Load A from address");
#define COMMENT_STC         strcpy (data.comment, "Set Carry flag");
#define COMMENT_CMC         strcpy (data.comment, "Complement Carry flag");
#define COMMENT_MOV(x, y)   sprintf(data.comment, "Copy %s to %s", y, x);
#define COMMENT_ADD(x)      sprintf(data.comment, "Add %s to A", x);
#define COMMENT_ADC(x)      sprintf(data.comment, "Add %s to A and set Carry flag", x);
#define COMMENT_ADI         strcpy (data.comment, "Add immediate to A");
#define COMMENT_ACI         strcpy (data.comment, "Add immediate to A and set Carry flag");
#define COMMENT_SUB(x)      sprintf(data.comment, "Subtract %s from A", x);
#define COMMENT_SBB(x)      sprintf(data.comment, "Subtract %s from A with borrow", x);
#define COMMENT_SUI         strcpy (data.comment, "Subtract immediate from A");
#define COMMENT_SBI         strcpy (data.comment, "Subtract immediate from A with borrow");
#define COMMENT_ANA(x)      sprintf(data.comment, "A <- %s AND A", x);
#define COMMENT_ANI         sprintf(data.comment, "A <- immediate AND A");
#define COMMENT_XRA(x)      sprintf(data.comment, "A <- %s XOR A", x);
#define COMMENT_XRI         sprintf(data.comment, "A <- immediate XOR A");
#define COMMENT_ORA(x)      sprintf(data.comment, "A <- %s OR A", x);
#define COMMENT_ORI         strcpy (data.comment, "A <- immediate OR A");
#define COMMENT_CMP(x)      sprintf(data.comment, "Compare %s with A and store result in Zero flag", x);
#define COMMENT_CPI         strcpy (data.comment, "Compare immediate with A and store result in Zero flag");
#define COMMENT_RET         strcpy (data.comment, "Return");
#define COMMENT_RZ          strcpy (data.comment, "Return if Zero flag is set (Equal)");
#define COMMENT_RNZ         strcpy (data.comment, "Return if Zero flag is not set (Not Equal)");
#define COMMENT_RC          strcpy (data.comment, "Return if Carry flag is set");
#define COMMENT_RNC         strcpy (data.comment, "Return if Carry flag is not set");
#define COMMENT_RPE         strcpy (data.comment, "Return if Parity flag is set (Even)");
#define COMMENT_RPO         strcpy (data.comment, "Return if Parity flag is not set (Odd)");
#define COMMENT_RP          strcpy (data.comment, "Return if Sign flag is not set (Plus)");
#define COMMENT_RM          strcpy (data.comment, "Return if Sign flag is set (Minus)");
#define COMMENT_JZ          strcpy (data.comment, "Jump if Zero flag is set");
#define COMMENT_JNZ         strcpy (data.comment, "Jump if Zero flag is not set");
#define COMMENT_JC          strcpy (data.comment, "Jump if Carry flag is set");
#define COMMENT_JNC         strcpy (data.comment, "Jump if Carry flag is not set");
#define COMMENT_JPE         strcpy (data.comment, "Jump if Parity flag is set");
#define COMMENT_JPO         strcpy (data.comment, "Jump if Parity flag is not set");
#define COMMENT_JP          strcpy (data.comment, "Jump if Sign flag is not set (positive result)");
#define COMMENT_JM          strcpy (data.comment, "Jump if Sign flag is set (negative result)");
#define COMMENT_CZ          strcpy (data.comment, "Call if Zero flag is set");
#define COMMENT_CNZ         strcpy (data.comment, "Call if Zero flag is not set");
#define COMMENT_CC          strcpy (data.comment, "Call if Carry flag is set");
#define COMMENT_CNC         strcpy (data.comment, "Call if Carry flag is not set");
#define COMMENT_CPE         strcpy (data.comment, "Call if Parity flag is set");
#define COMMENT_CPO         strcpy (data.comment, "Call if Parity flag is not set");
#define COMMENT_CP          strcpy (data.comment, "Call if Sign flag is not set (positive result)");
#define COMMENT_CM          strcpy (data.comment, "Call if Sign flag is set (negative result)");
#define COMMENT_RST(x)      sprintf(data.comment, "Call address %04Xh", x);
#define COMMENT_PCHL        strcpy (data.comment, "Jump to $HL");
#define COMMENT_XTHL        strcpy (data.comment, "Swap HL with the top word of the Stack");
#define COMMENT_SPHL        strcpy (data.comment, "Copy HL to the Stack Pointer");
#define COMMENT_XCHG        strcpy (data.comment, "Swap the contents of HL and DE");
#define COMMENT_HLT         strcpy (data.comment, "Halt processor");
#define COMMENT_IN          strcpy (data.comment, "Read input port to A");
#define COMMENT_OUT         strcpy (data.comment, "Write A to output port");
#define COMMENT_EI          strcpy (data.comment, "Enable Interrupts");
#define COMMENT_DI          strcpy (data.comment, "Disable Interrupts");



/* ---------------- DISASSEMBLE_INSTRUCTION ---------------- */


// raccoglie tutte le info sulla istruzione fornita e le restituisce senza stamparle
i8080_instruction_t disassemble_instruction( const uint8_t* mem , unsigned int addr ) {
    i8080_instruction_t data;
    
    data.opcode = mem[addr];
    
    data.position = addr;
    data.num_inputRegisters = 0;     //
    data.num_inputValues = 0;        //
    data.instructionLength = 1;      // default values
    data.immediate = true;           //
    memset(data.comment, 0, I8080_MAX_COMMENT_LENGTH);
    
    switch (data.opcode) {
        case 0x00: {
            strcpy(data.mnemonic,"NOP ");
            data.instructionLength = 1;
            break;
        }
        case 0x01: {
            strcpy(data.mnemonic, "LXI ");
            strcpy(data.inputRegisters, "BC");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            
            data.instructionLength = 3;
            data.num_inputRegisters = 1;
            data.num_inputValues = 2;

            COMMENT_LXI("BC")
            break;
        }
        case 0x02: {
            strcpy(data.mnemonic, "STAX");
            strcpy(data.inputRegisters, "$BC");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_STAX("BC")
            break;
        }
        case 0x03: {
            strcpy(data.mnemonic, "INX ");
            strcpy(data.inputRegisters, "BC");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_INX("BC")
            break;
        }
        case 0x04: {
            strcpy(data.mnemonic, "INR ");
            strcpy(data.inputRegisters, "B");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_INR("B")
            break;
        }
        case 0x05: {
            strcpy(data.mnemonic, "DCR ");
            strcpy(data.inputRegisters, "B");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_DCR("B")
            break;
        }
        case 0x06: {
            strcpy(data.mnemonic, "MVI ");
            strcpy(data.inputRegisters, "B");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 1;
            data.num_inputValues = 1;

            COMMENT_MVI("B")
            break;
        }
        case 0x07: {
            strcpy(data.mnemonic,"RLC ");
            data.instructionLength = 1;

            COMMENT_RLC
            break;
        }
        case 0x08: {
            strcpy(data.mnemonic,"NOP ");
            data.instructionLength = 1;
            break;
        }
        case 0x09: {
            strcpy(data.mnemonic, "DAD ");
            strcpy(data.inputRegisters, "BC");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_DAD("BC")
            break;
        }
        case 0x0A: {
            strcpy(data.mnemonic, "LDAX");
            strcpy(data.inputRegisters, "$BC");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_LDAX("BC")
            break;
        }
        case 0x0B: {
            strcpy(data.mnemonic, "DCX ");
            strcpy(data.inputRegisters, "BC");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_DCX("BC")
            break;
        }
        case 0x0C: {
            strcpy(data.mnemonic, "INR ");
            strcpy(data.inputRegisters, "C");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_INR("C")
            break;
        }
        case 0x0D: {
            strcpy(data.mnemonic, "DCR ");
            strcpy(data.inputRegisters, "C");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_DCR("C")
            break;
        }
        case 0x0E: {
            strcpy(data.mnemonic, "MVI ");
            strcpy(data.inputRegisters, "C");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 1;
            data.num_inputValues = 1;

            COMMENT_MVI("C")
            break;
        }
        case 0x0F: {
            strcpy(data.mnemonic, "RRC ");
            
            data.instructionLength = 1;

            COMMENT_RRC
            break;
        }
        case 0x10: {
            strcpy(data.mnemonic, "NOP ");
            
            data.instructionLength = 1;
            break;
        }
        case 0x11: {
            strcpy(data.mnemonic, "LXI ");
            strcpy(data.inputRegisters, "DE");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            
            data.instructionLength = 3;
            data.num_inputRegisters = 1;
            data.num_inputValues = 2;

            COMMENT_LXI("DE")
            break;
        }
        case 0x12: {
            strcpy(data.mnemonic, "STAX");
            strcpy(data.inputRegisters, "DE");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_STAX("DE")
            break;
        }
        case 0x13: {
            strcpy(data.mnemonic, "INX ");
            strcpy(data.inputRegisters, "DE");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_INX("DE")
            break;
        }
        case 0x14: {
            strcpy(data.mnemonic, "INR ");
            strcpy(data.inputRegisters, "D");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_INR("D")
            break;
        }
        case 0x15: {
            strcpy(data.mnemonic, "DCR ");
            strcpy(data.inputRegisters, "D");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_DCR("D")
            break;
        }
        case 0x16: {
            strcpy(data.mnemonic, "MVI ");
            strcpy(data.inputRegisters, "D");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 1;
            data.num_inputValues = 1;

            COMMENT_MVI("D")
            break;
        }
        case 0x17: {
            strcpy(data.mnemonic,"RAL ");
            data.instructionLength = 1;

            COMMENT_RAL
            break;
        }
        case 0x18: {
            strcpy(data.mnemonic,"NOP ");
            data.instructionLength = 1;
            break;
        }
        case 0x19: {
            strcpy(data.mnemonic, "DAD ");
            strcpy(data.inputRegisters, "DE");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_DAD("DE")
            break;
        }
        case 0x1A: {
            strcpy(data.mnemonic, "LDAX");
            strcpy(data.inputRegisters, "DE");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_LDAX("DE")
            break;
        }
        case 0x1B: {
            strcpy(data.mnemonic, "DCX ");
            strcpy(data.inputRegisters, "DE");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_DCX("DE")
            break;
        }
        case 0x1C: {
            strcpy(data.mnemonic, "INR ");
            strcpy(data.inputRegisters, "E");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_INR("E")
            break;
        }
        case 0x1D: {
            strcpy(data.mnemonic, "DCR ");
            strcpy(data.inputRegisters, "E");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_DCR("E")
            break;
        }
        case 0x1E: {
            strcpy(data.mnemonic, "MVI ");
            strcpy(data.inputRegisters, "E");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 1;
            data.num_inputValues = 1;

            COMMENT_MVI("E")
            break;
        }
        case 0x1F: {
            strcpy(data.mnemonic, "RAR ");
            
            data.instructionLength = 1;

            COMMENT_RAR
            break;
        }
        case 0x20: {
            strcpy(data.mnemonic, "NOP ");
            
            data.instructionLength = 1;
            break;
        }
        case 0x21: {
            strcpy(data.mnemonic, "LXI ");
            strcpy(data.inputRegisters, "HL");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            
            data.instructionLength = 3;
            data.num_inputRegisters = 1;
            data.num_inputValues = 2;

            COMMENT_LXI("HL")
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

            COMMENT_SHLD
            break;
        }
        case 0x23: {
            strcpy(data.mnemonic, "INX ");
            strcpy(data.inputRegisters, "HL");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_INX("HL")
            break;
        }
        case 0x24: {
            strcpy(data.mnemonic, "INR ");
            strcpy(data.inputRegisters, "H");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_INR("H")
            break;
        }
        case 0x25: {
            strcpy(data.mnemonic, "DCR ");
            strcpy(data.inputRegisters, "H");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_DCR("H")
            break;
        }
        case 0x26: {
            strcpy(data.mnemonic, "MVI ");
            strcpy(data.inputRegisters, "H");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 1;
            data.num_inputValues = 1;

            COMMENT_MVI("H")
            break;
        }
        case 0x27: {
            strcpy(data.mnemonic,"DAA ");
            data.instructionLength = 1;

            COMMENT_DAA
            break;
        }
        case 0x28: {
            strcpy(data.mnemonic,"NOP ");
            data.instructionLength = 1;
            break;
        }
        case 0x29: {
            strcpy(data.mnemonic, "DAD ");
            strcpy(data.inputRegisters, "HL");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_DAD("HL")
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

            COMMENT_LHLD
            break;
        }
        case 0x2B: {
            strcpy(data.mnemonic, "DCX ");
            strcpy(data.inputRegisters, "HL");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_DCX("HL")
            break;
        }
        case 0x2C: {
            strcpy(data.mnemonic, "INR ");
            strcpy(data.inputRegisters, "L");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_INR("L")
            break;
        }
        case 0x2D: {
            strcpy(data.mnemonic, "DCR ");
            strcpy(data.inputRegisters, "L");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_DCR("L")
            break;
        }
        case 0x2E: {
            strcpy(data.mnemonic, "MVI ");
            strcpy(data.inputRegisters, "L");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 1;
            data.num_inputValues = 1;

            COMMENT_MVI("L")
            break;
        }
        case 0x2F: {
            strcpy(data.mnemonic, "CMA ");
            
            data.instructionLength = 1;

            COMMENT_CMA
            break;
        }
        case 0x30: {
            strcpy(data.mnemonic, "NOP ");
            
            data.instructionLength = 1;
            break;
        }
        case 0x31: {
            strcpy(data.mnemonic, "LXI ");
            strcpy(data.inputRegisters, "SP");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            
            data.instructionLength = 3;
            data.num_inputRegisters = 1;
            data.num_inputValues = 2;

            COMMENT_LXI("the Stack Pointer")
            break;
        }
        case 0x32: {
            strcpy(data.mnemonic, "STA ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputRegisters = 0;
            data.num_inputValues = 2;

            COMMENT_STA
            break;
        }
        case 0x33: {
            strcpy(data.mnemonic, "INX ");
            strcpy(data.inputRegisters, "SP");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_INX("the Stack Pointer")
            break;
        }
        case 0x34: {
            strcpy(data.mnemonic, "INR ");
            strcpy(data.inputRegisters, "$HL");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_INR("$HL (indirect)")
            break;
        }
        case 0x35: {
            strcpy(data.mnemonic, "DCR ");
            strcpy(data.inputRegisters, "$HL");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_DCR("$HL (indirect)")
            break;
        }
        case 0x36: {
            strcpy(data.mnemonic, "MVI ");
            strcpy(data.inputRegisters, "$HL");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 1;
            data.num_inputValues = 1;

            COMMENT_MVI("$HL (indirect)")
            break;
        }
        case 0x37: {
            strcpy(data.mnemonic,"STC ");
            
            data.instructionLength = 1;

            COMMENT_STC
            break;
        }
        case 0x38: {
            strcpy(data.mnemonic,"NOP ");
            
            data.instructionLength = 1;
            break;
        }
        case 0x39: {
            strcpy(data.mnemonic, "DAD ");
            strcpy(data.inputRegisters, "SP");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_DAD("the Stack Pointer")
            break;
        }
        case 0x3A: {
            strcpy(data.mnemonic, "LDA ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputRegisters = 0;
            data.num_inputValues = 2;

            COMMENT_LDA
            break;
        }
        case 0x3B: {
            strcpy(data.mnemonic, "DCX ");
            strcpy(data.inputRegisters, "SP");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_DCX("the Stack Pointer")
            break;
        }
        case 0x3C: {
            strcpy(data.mnemonic, "INR ");
            strcpy(data.inputRegisters, "A");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_INR("A")
            break;
        }
        case 0x3D: {
            strcpy(data.mnemonic, "DCR ");
            strcpy(data.inputRegisters, "A");
            
            data.instructionLength = 1;
            data.num_inputRegisters = 1;

            COMMENT_DCR("A")
            break;
        }
        case 0x3E: {
            strcpy(data.mnemonic, "MVI ");
            strcpy(data.inputRegisters, "A");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 1;
            data.num_inputValues = 1;

            COMMENT_MVI("A")
            break;
        }
        case 0x3F: {
            strcpy(data.mnemonic, "CMC ");
            
            data.instructionLength = 1;

            COMMENT_CMC
            break;
        }
        case 0x40: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "B, B");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("B","B")
            break;
        }
        case 0x41: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "B, C");
            
            data.num_inputRegisters = 2;
            COMMENT_MOV("B","C")
            break;
        }
        case 0x42: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "B, D");
            
            data.num_inputRegisters = 2;
            COMMENT_MOV("B","D")
            break;
        }
        case 0x43: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "B, E");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("B","E")
            break;
        }
        case 0x44: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "B, H");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("B","H")
            break;
        }
        case 0x45: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "B, L");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("B", "L")
            break;
        }
        case 0x46: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "B, $HL");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("B","$HL")
            break;
        }
        case 0x47: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "B, A");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("B","A")
            break;
        }
        case 0x48: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "C, B");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("C","B")
            break;
        }
        case 0x49: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "C, C");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("C","C")
            break;
        }
        case 0x4A: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "C, D");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("C","D")
            break;
        }
        case 0x4B: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "C, E");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("C","E")
            break;
        }
        case 0x4C: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "C, H");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("C","H")
            break;
        }
        case 0x4D: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "C, L");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("C","L")
            break;
        }
        case 0x4E: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "C, $HL");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("C","$HL")
            break;
        }
        case 0x4F: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "C, A");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("C","A")
            break;
        }
        case 0x50: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "D, B");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("D","B")
            break;
        }
        case 0x51: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "D, C");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("D","C")
            break;
        }
        case 0x52: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "D, D");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("D","D")
            break;
        }
        case 0x53: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "D, E");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("D","E")
            break;
        }
        case 0x54: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "D, H");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("D","H")
            break;
        }
        case 0x55: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "D, L");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("D","L")
            break;
        }
        case 0x56: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "D, $HL");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("D","$HL")
            break;
        }
        case 0x57: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "D, A");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("D","A")
            break;
        }
        case 0x58: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "E, B");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("E","B")
            break;
        }
        case 0x59: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "E, C");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("E","C")
            break;
        }
        case 0x5A: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "E, D");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("E","D")
            break;
        }
        case 0x5B: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "E, E");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("E","E")
            break;
        }
        case 0x5C: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "E, H");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("E","H")
            break;
        }
        case 0x5D: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "E, L");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("E","L")
            break;
        }
        case 0x5E: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "E, $HL");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("E","$HL")
            break;
        }
        case 0x5F: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "E, A");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("E","A")
            break;
        }
        case 0x60: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "H, B");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("H","B")
            break;
        }
        case 0x61: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "H, C");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("H","C")
            break;
        }
        case 0x62: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "H, D");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("H","D")
            break;
        }
        case 0x63: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "H, E");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("H","E")
            break;
        }
        case 0x64: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "H, H");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("H","H")
            break;
        }
        case 0x65: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "H, L");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("H","L")
            break;
        }
        case 0x66: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "H, $HL");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("H","$HL")
            break;
        }
        case 0x67: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "H, A");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("H","A")
            break;
        }
        case 0x68: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "L, B");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("L","B")
            break;
        }
        case 0x69: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "L, C");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("L","C")
            break;
        }
        case 0x6A: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "L, D");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("L","D")
            break;
        }
        case 0x6B: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "L, E");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("L","E")
            break;
        }
        case 0x6C: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "L, H");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("L","H")
            break;
        }
        case 0x6D: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "L, L");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("L","L")
            break;
        }
        case 0x6E: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "L, $HL");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("L","$HL")
            break;
        }
        case 0x6F: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "L, A");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("L","A")
            break;
        }
        case 0x70: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "$HL, B");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("$HL","B")
            break;
        }
        case 0x71: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "$HL, C");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("$HL","C")
            break;
        }
        case 0x72: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "$HL, D");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("$HL","D")
            break;
        }
        case 0x73: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "$HL, E");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("$HL","E")
            break;
        }
        case 0x74: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "$HL, H");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("$HL","H")
            break;
        }
        case 0x75: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "$HL, L");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("$HL","L")
            break;
        }
        case 0x76: {
            strcpy(data.mnemonic, "HLT ");
            
            data.instructionLength = 1;

            COMMENT_HLT
            break;
        }
        case 0x77: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "$HL, A");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("$HL","A")
            break;
        }
        case 0x78: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "A, B");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("A","B")
            break;
        }
        case 0x79: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "A, C");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("A","C")
            break;
        }
        case 0x7A: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "A, D");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("A","D")
            break;
        }
        case 0x7B: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "A, E");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("A","E")
            break;
        }
        case 0x7C: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "A, H");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("A","H")
            break;
        }
        case 0x7D: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "A, L");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("A","L")
            break;
        }
        case 0x7E: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "A, $HL");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("A","$HL")
            break;
        }
        case 0x7F: {
            strcpy(data.mnemonic, "MOV ");
            strcpy(data.inputRegisters, "A, A");
            
            data.num_inputRegisters = 2;

            COMMENT_MOV("A","A")
            break;
        }
        case 0x80: {
            strcpy(data.mnemonic, "ADD ");
            strcpy(data.inputRegisters, "B");
            
            data.num_inputRegisters = 1;

            COMMENT_ADD("B")
            break;
        }
        case 0x81: {
            strcpy(data.mnemonic, "ADD ");
            strcpy(data.inputRegisters, "C");
            
            data.num_inputRegisters = 1;

            COMMENT_ADD("C")
            break;
        }
        case 0x82: {
            strcpy(data.mnemonic, "ADD ");
            strcpy(data.inputRegisters, "D");
            
            data.num_inputRegisters = 1;

            COMMENT_ADD("D")
            break;
        }
        case 0x83: {
            strcpy(data.mnemonic, "ADD ");
            strcpy(data.inputRegisters, "E");
            
            data.num_inputRegisters = 1;

            COMMENT_ADD("E")
            break;
        }
        case 0x84: {
            strcpy(data.mnemonic, "ADD ");
            strcpy(data.inputRegisters, "H");
            
            data.num_inputRegisters = 1;

            COMMENT_ADD("H")
            break;
        }
        case 0x85: {
            strcpy(data.mnemonic, "ADD ");
            strcpy(data.inputRegisters, "L");
            
            data.num_inputRegisters = 1;

            COMMENT_ADD("L")
            break;
        }
        case 0x86: {
            strcpy(data.mnemonic, "ADD ");
            strcpy(data.inputRegisters, "$HL");
            
            data.num_inputRegisters = 1;

            COMMENT_ADD("$HL")
            break;
        }
        case 0x87: {
            strcpy(data.mnemonic, "ADD ");
            strcpy(data.inputRegisters, "A");
            
            data.num_inputRegisters = 1;

            COMMENT_ADD("A")
            break;
        }
        case 0x88: {
            strcpy(data.mnemonic, "ADC ");
            strcpy(data.inputRegisters, "B");
            
            data.num_inputRegisters = 1;

            COMMENT_ADC("B")
            break;
        }
        case 0x89: {
            strcpy(data.mnemonic, "ADC ");
            strcpy(data.inputRegisters, "C");
            
            data.num_inputRegisters = 1;

            COMMENT_ADC("C")
            break;
        }
        case 0x8A: {
            strcpy(data.mnemonic, "ADC ");
            strcpy(data.inputRegisters, "D");
            
            data.num_inputRegisters = 1;

            COMMENT_ADC("D")
            break;
        }
        case 0x8B: {
            strcpy(data.mnemonic, "ADC ");
            strcpy(data.inputRegisters, "E");
            
            data.num_inputRegisters = 1;

            COMMENT_ADC("E")
            break;
        }
        case 0x8C: {
            strcpy(data.mnemonic, "ADC ");
            strcpy(data.inputRegisters, "H");
            
            data.num_inputRegisters = 1;

            COMMENT_ADC("H")
            break;
        }
        case 0x8D: {
            strcpy(data.mnemonic, "ADC ");
            strcpy(data.inputRegisters, "L");
            
            data.num_inputRegisters = 1;

            COMMENT_ADC("L")
            break;
        }
        case 0x8E: {
            strcpy(data.mnemonic, "ADC ");
            strcpy(data.inputRegisters, "$HL");
            
            data.num_inputRegisters = 1;

            COMMENT_ADC("$HL")
            break;
        }
        case 0x8F: {
            strcpy(data.mnemonic, "ADC ");
            strcpy(data.inputRegisters, "A");
            
            data.num_inputRegisters = 1;

            COMMENT_ADC("A")
            break;
        }
        case 0x90: {
            strcpy(data.mnemonic, "SUB ");
            strcpy(data.inputRegisters, "B");
            
            data.num_inputRegisters = 1;

            COMMENT_SUB("B")
            break;
        }
        case 0x91: {
            strcpy(data.mnemonic, "SUB ");
            strcpy(data.inputRegisters, "C");
            
            data.num_inputRegisters = 1;

            COMMENT_SUB("C")
            break;
        }
        case 0x92: {
            strcpy(data.mnemonic, "SUB ");
            strcpy(data.inputRegisters, "D");
            
            data.num_inputRegisters = 1;

            COMMENT_SUB("D")
            break;
        }
        case 0x93: {
            strcpy(data.mnemonic, "SUB ");
            strcpy(data.inputRegisters, "E");
            
            data.num_inputRegisters = 1;

            COMMENT_SUB("E")
            break;
        }
        case 0x94: {
            strcpy(data.mnemonic, "SUB ");
            strcpy(data.inputRegisters, "H");
            
            data.num_inputRegisters = 1;

            COMMENT_SUB("H")
            break;
        }
        case 0x95: {
            strcpy(data.mnemonic, "SUB ");
            strcpy(data.inputRegisters, "L");
            
            data.num_inputRegisters = 1;

            COMMENT_SUB("L")
            break;
        }
        case 0x96: {
            strcpy(data.mnemonic, "SUB ");
            strcpy(data.inputRegisters, "$HL");
            
            data.num_inputRegisters = 1;

            COMMENT_SUB("$HL")
            break;
        }
        case 0x97: {
            strcpy(data.mnemonic, "SUB ");
            strcpy(data.inputRegisters, "A");
            
            data.num_inputRegisters = 1;

            COMMENT_SUB("A")
            break;
        }
        case 0x98: {
            strcpy(data.mnemonic, "SBB ");
            strcpy(data.inputRegisters, "B");
            
            data.num_inputRegisters = 1;

            COMMENT_SBB("B")
            break;
        }
        case 0x99: {
            strcpy(data.mnemonic, "SBB ");
            strcpy(data.inputRegisters, "C");
            
            data.num_inputRegisters = 1;

            COMMENT_SBB("C")
            break;
        }
        case 0x9A: {
            strcpy(data.mnemonic, "SBB ");
            strcpy(data.inputRegisters, "D");
            
            data.num_inputRegisters = 1;

            COMMENT_SBB("D")
            break;
        }
        case 0x9B: {
            strcpy(data.mnemonic, "SBB ");
            strcpy(data.inputRegisters, "E");
            
            data.num_inputRegisters = 1;

            COMMENT_SBB("E")
            break;
        }
        case 0x9C: {
            strcpy(data.mnemonic, "SBB ");
            strcpy(data.inputRegisters, "H");
            
            data.num_inputRegisters = 1;

            COMMENT_SBB("H")
            break;
        }
        case 0x9D: {
            strcpy(data.mnemonic, "SBB ");
            strcpy(data.inputRegisters, "L");
            
            data.num_inputRegisters = 1;

            COMMENT_SBB("L")
            break;
        }
        case 0x9E: {
            strcpy(data.mnemonic, "SBB ");
            strcpy(data.inputRegisters, "$HL");
            
            data.num_inputRegisters = 1;

            COMMENT_SBB("$HL")
            break;
        }
        case 0x9F: {
            strcpy(data.mnemonic, "SBB ");
            strcpy(data.inputRegisters, "A");
            
            data.num_inputRegisters = 1;

            COMMENT_SBB("A")
            break;
        }
        case 0xA0: {
            strcpy(data.mnemonic, "ANA ");
            strcpy(data.inputRegisters, "B");
            
            data.num_inputRegisters = 1;

            COMMENT_ANA("B")
            break;
        }
        case 0xA1: {
            strcpy(data.mnemonic, "ANA ");
            strcpy(data.inputRegisters, "C");
            
            data.num_inputRegisters = 1;

            COMMENT_ANA("C")
            break;
        }
        case 0xA2: {
            strcpy(data.mnemonic, "ANA ");
            strcpy(data.inputRegisters, "D");
            
            data.num_inputRegisters = 1;

            COMMENT_ANA("D")
            break;
        }
        case 0xA3: {
            strcpy(data.mnemonic, "ANA ");
            strcpy(data.inputRegisters, "E");
            
            data.num_inputRegisters = 1;

            COMMENT_ANA("E")
            break;
        }
        case 0xA4: {
            strcpy(data.mnemonic, "ANA ");
            strcpy(data.inputRegisters, "H");
            
            data.num_inputRegisters = 1;

            COMMENT_ANA("H")
            break;
        }
        case 0xA5: {
            strcpy(data.mnemonic, "ANA ");
            strcpy(data.inputRegisters, "L");
            
            data.num_inputRegisters = 1;

            COMMENT_ANA("L")
            break;
        }
        case 0xA6: {
            strcpy(data.mnemonic, "ANA ");
            strcpy(data.inputRegisters, "$HL");
            
            data.num_inputRegisters = 1;

            COMMENT_ANA("$HL")
            break;
        }
        case 0xA7: {
            strcpy(data.mnemonic, "ANA ");
            strcpy(data.inputRegisters, "A");
            
            data.num_inputRegisters = 1;

            COMMENT_ANA("A")
            break;
        }
        case 0xA8: {
            strcpy(data.mnemonic, "XRA ");
            strcpy(data.inputRegisters, "B");
            
            data.num_inputRegisters = 1;

            COMMENT_XRA("B")
            break;
        }
        case 0xA9: {
            strcpy(data.mnemonic, "XRA ");
            strcpy(data.inputRegisters, "C");
            
            data.num_inputRegisters = 1;

            COMMENT_XRA("C")
            break;
        }
        case 0xAA: {
            strcpy(data.mnemonic, "XRA ");
            strcpy(data.inputRegisters, "D");
            
            data.num_inputRegisters = 1;

            COMMENT_XRA("D")
            break;
        }
        case 0xAB: {
            strcpy(data.mnemonic, "XRA ");
            strcpy(data.inputRegisters, "E");
            
            data.num_inputRegisters = 1;

            COMMENT_XRA("E")
            break;
        }
        case 0xAC: {
            strcpy(data.mnemonic, "XRA ");
            strcpy(data.inputRegisters, "H");
            
            data.num_inputRegisters = 1;

            COMMENT_XRA("H")
            break;
        }
        case 0xAD: {
            strcpy(data.mnemonic, "XRA ");
            strcpy(data.inputRegisters, "L");
            
            data.num_inputRegisters = 1;

            COMMENT_XRA("L")
            break;
        }
        case 0xAE: {
            strcpy(data.mnemonic, "XRA ");
            strcpy(data.inputRegisters, "$HL");
            
            data.num_inputRegisters = 1;

            COMMENT_XRA("$HL")
            break;
        }
        case 0xAF: {
            strcpy(data.mnemonic, "XRA ");
            strcpy(data.inputRegisters, "A");
            
            data.num_inputRegisters = 1;

            COMMENT_XRA("A")
            break;
        }
        case 0xB0: {
            strcpy(data.mnemonic, "ORA ");
            strcpy(data.inputRegisters, "B");
            
            data.num_inputRegisters = 1;

            COMMENT_ORA("B")
            break;
        }
        case 0xB1: {
            strcpy(data.mnemonic, "ORA ");
            strcpy(data.inputRegisters, "C");
            
            data.num_inputRegisters = 1;

            COMMENT_ORA("C")
            break;
        }
        case 0xB2: {
            strcpy(data.mnemonic, "ORA ");
            strcpy(data.inputRegisters, "D");
            
            data.num_inputRegisters = 1;

            COMMENT_ORA("D")
            break;
        }
        case 0xB3: {
            strcpy(data.mnemonic, "ORA ");
            strcpy(data.inputRegisters, "E");
            
            data.num_inputRegisters = 1;

            COMMENT_ORA("E")
            break;
        }
        case 0xB4: {
            strcpy(data.mnemonic, "ORA ");
            strcpy(data.inputRegisters, "H");
            
            data.num_inputRegisters = 1;

            COMMENT_ORA("H")
            break;
        }
        case 0xB5: {
            strcpy(data.mnemonic, "ORA ");
            strcpy(data.inputRegisters, "L");
            
            data.num_inputRegisters = 1;

            COMMENT_ORA("L")
            break;
        }
        case 0xB6: {
            strcpy(data.mnemonic, "ORA ");
            strcpy(data.inputRegisters, "$HL");
            
            data.num_inputRegisters = 1;

            COMMENT_ORA("$HL")
            break;
        }
        case 0xB7: {
            strcpy(data.mnemonic, "ORA ");
            strcpy(data.inputRegisters, "A");
            
            data.num_inputRegisters = 1;

            COMMENT_ORA("A")
            break;
        }
        case 0xB8: {
            strcpy(data.mnemonic, "CMP ");
            strcpy(data.inputRegisters, "B");
            
            data.num_inputRegisters = 1;

            COMMENT_CMP("B")
            break;
        }
        case 0xB9: {
            strcpy(data.mnemonic, "CMP ");
            strcpy(data.inputRegisters, "C");
            
            data.num_inputRegisters = 1;

            COMMENT_CMP("C")
            break;
        }
        case 0xBA: {
            strcpy(data.mnemonic, "CMP ");
            strcpy(data.inputRegisters, "D");
            
            data.num_inputRegisters = 1;

            COMMENT_CMP("D")
            break;
        }
        case 0xBB: {
            strcpy(data.mnemonic, "CMP ");
            strcpy(data.inputRegisters, "E");
            
            data.num_inputRegisters = 1;

            COMMENT_CMP("E")
            break;
        }
        case 0xBC: {
            strcpy(data.mnemonic, "CMP ");
            strcpy(data.inputRegisters, "H");
            
            data.num_inputRegisters = 1;

            COMMENT_CMP("H")
            break;
        }
        case 0xBD: {
            strcpy(data.mnemonic, "CMP ");
            strcpy(data.inputRegisters, "L");
            
            data.num_inputRegisters = 1;

            COMMENT_CMP("L")
            break;
        }
        case 0xBE: {
            strcpy(data.mnemonic, "CMP ");
            strcpy(data.inputRegisters, "$HL");
            
            data.num_inputRegisters = 1;

            COMMENT_CMP("$HL")
            break;
        }
        case 0xBF: {
            strcpy(data.mnemonic, "CMP ");
            strcpy(data.inputRegisters, "A");
            
            data.num_inputRegisters = 1;

            COMMENT_CMP("A")
            break;
        }
        case 0xC0: {
            strcpy(data.mnemonic, "RNZ ");
            
            COMMENT_RNZ
            break;
        }
        case 0xC1: {
            strcpy(data.mnemonic, "POP ");
            strcpy(data.inputRegisters, "B");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xC2: {
            strcpy(data.mnemonic, "JNZ ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;

            COMMENT_JNZ
            break;
        }
        case 0xC3: {
            strcpy(data.mnemonic, "JMP ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xC4: {
            strcpy(data.mnemonic, "CNZ ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;

            COMMENT_CNZ
            break;
        }
        case 0xC5: {
            strcpy(data.mnemonic, "PUSH");
            strcpy(data.inputRegisters, "B");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xC6: {
            strcpy(data.mnemonic, "ADI ");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 0;
            data.num_inputValues = 1;

            COMMENT_ADI
            break;
        }
        case 0xC7: {
            strcpy(data.mnemonic, "RST ");
            strcpy(data.inputRegisters, "0");
            
            data.num_inputRegisters = 1;

            COMMENT_RST(0)
            break;
        }
        case 0xC8: {
            strcpy(data.mnemonic, "RZ  ");
            
            COMMENT_RZ
            break;
        }
        case 0xC9: {
            strcpy(data.mnemonic, "RET ");
            
            COMMENT_RET
            break;
        }
        case 0xCA: {
            strcpy(data.mnemonic, "JZ  ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;

            COMMENT_JZ
            break;
        }
        case 0xCB: {
            strcpy(data.mnemonic, "JMP ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;
            break;
        }
        case 0xCC: {
            strcpy(data.mnemonic, "CZ  ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;

            COMMENT_CZ
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
            strcpy(data.mnemonic, "ACI ");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputValues = 1;

            COMMENT_ACI
            break;
        }
        case 0xCF: {
            strcpy(data.mnemonic, "RST ");
            strcpy(data.inputRegisters, "1");
            
            data.num_inputRegisters = 1;

            COMMENT_RST(1)
            break;
        }
        case 0xD0: {
            strcpy(data.mnemonic, "RNC ");
            
            COMMENT_RNC
            break;
        }
        case 0xD1: {
            strcpy(data.mnemonic, "POP ");
            strcpy(data.inputRegisters, "D");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xD2: {
            strcpy(data.mnemonic, "JNC ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;

            COMMENT_JNC
            break;
        }
        case 0xD3: {
            strcpy(data.mnemonic, "OUT ");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputValues = 1;

            COMMENT_OUT
            break;
        }
        case 0xD4: {
            strcpy(data.mnemonic, "CNC ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;

            COMMENT_CNC
            break;
        }
        case 0xD5: {
            strcpy(data.mnemonic, "PUSH");
            strcpy(data.inputRegisters, "D");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xD6: {
            strcpy(data.mnemonic, "SUI ");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 0;
            data.num_inputValues = 1;

            COMMENT_SUI
            break;
        }
        case 0xD7: {
            strcpy(data.mnemonic, "RST ");
            strcpy(data.inputRegisters, "2");
            
            data.num_inputRegisters = 1;

            COMMENT_RST(2)
            break;
        }
        case 0xD8: {
            strcpy(data.mnemonic, "RC  ");
            
            COMMENT_RC
            break;
        }
        case 0xD9: {
            strcpy(data.mnemonic, "RET ");
            
            COMMENT_RET
            break;
        }
        case 0xDA: {
            strcpy(data.mnemonic, "JC  ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;

            COMMENT_JC
            break;
        }
        case 0xDB: {
            strcpy(data.mnemonic, "IN  ");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputValues = 1;

            COMMENT_IN
            break;
        }
        case 0xDC: {
            strcpy(data.mnemonic, "CC  ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;

            COMMENT_CC
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
            strcpy(data.mnemonic, "SBI ");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputValues = 1;

            COMMENT_SBI
            break;
        }
        case 0xDF: {
            strcpy(data.mnemonic, "RST ");
            strcpy(data.inputRegisters, "3");
            
            data.num_inputRegisters = 1;

            COMMENT_RST(3)
            break;
        }
        case 0xE0: {
            strcpy(data.mnemonic, "RPO ");
            
            COMMENT_RPO
            break;
        }
        case 0xE1: {
            strcpy(data.mnemonic, "POP ");
            strcpy(data.inputRegisters, "H");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xE2: {
            strcpy(data.mnemonic, "JPO ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;

            COMMENT_JPO
            break;
        }
        case 0xE3: {
            strcpy(data.mnemonic, "XTHL");
            
            COMMENT_XTHL
            break;
        }
        case 0xE4: {
            strcpy(data.mnemonic, "CPO ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;

            COMMENT_CPO
            break;
        }
        case 0xE5: {
            strcpy(data.mnemonic, "PUSH");
            strcpy(data.inputRegisters, "H");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xE6: {
            strcpy(data.mnemonic, "ANI ");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 0;
            data.num_inputValues = 1;

            COMMENT_ANI
            break;
        }
        case 0xE7: {
            strcpy(data.mnemonic, "RST ");
            strcpy(data.inputRegisters, "4");
            
            data.num_inputRegisters = 1;

            COMMENT_RST(4)
            break;
        }
        case 0xE8: {
            strcpy(data.mnemonic, "RPE ");
            
            COMMENT_RPE
            break;
        }
        case 0xE9: {
            strcpy(data.mnemonic, "PCHL");
            
            COMMENT_PCHL
            break;
        }
        case 0xEA: {
            strcpy(data.mnemonic, "JPE ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;

            COMMENT_JPE
            break;
        }
        case 0xEB: {
            strcpy(data.mnemonic, "XCHG");
            
            COMMENT_XCHG
            break;
        }
        case 0xEC: {
            strcpy(data.mnemonic, "CPE ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;

            COMMENT_CPE
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
            strcpy(data.mnemonic, "XRI ");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputValues = 1;

            COMMENT_XRI
            break;
        }
        case 0xEF: {
            strcpy(data.mnemonic, "RST ");
            strcpy(data.inputRegisters, "5");
            
            data.num_inputRegisters = 1;

            COMMENT_RST(5)
            break;
        }
        case 0xF0: {
            strcpy(data.mnemonic, "RP  ");
            
            COMMENT_RP
            break;
        }
        case 0xF1: {
            strcpy(data.mnemonic, "POP ");
            strcpy(data.inputRegisters, "PSW ");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xF2: {
            strcpy(data.mnemonic, "JP  ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;

            COMMENT_JP
            break;
        }
        case 0xF3: {
            strcpy(data.mnemonic, "DI  ");
            
            COMMENT_DI
            break;
        }
        case 0xF4: {
            strcpy(data.mnemonic, "CP  ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;

            COMMENT_CP
            break;
        }
        case 0xF5: {
            strcpy(data.mnemonic, "PUSH");
            strcpy(data.inputRegisters, "PSW ");
            
            data.num_inputRegisters = 1;
            break;
        }
        case 0xF6: {
            strcpy(data.mnemonic, "ORI ");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputRegisters = 0;
            data.num_inputValues = 1;

            COMMENT_ORI
            break;
        }
        case 0xF7: {
            strcpy(data.mnemonic, "RST ");
            strcpy(data.inputRegisters, "6");
            
            data.num_inputRegisters = 1;

            COMMENT_RST(6)
            break;
        }
        case 0xF8: {
            strcpy(data.mnemonic, "RM  ");
            
            COMMENT_RM
            break;
        }
        case 0xF9: {
            strcpy(data.mnemonic, "SPHL");
            
            COMMENT_SPHL
            break;
        }
        case 0xFA: {
            strcpy(data.mnemonic, "JM  ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;

            COMMENT_JM
            break;
        }
        case 0xFB: {
            strcpy(data.mnemonic, "EI  ");
            
            COMMENT_EI
            break;
        }
        case 0xFC: {
            strcpy(data.mnemonic, "CM  ");
            
            data.inputValues[0] = mem[addr+1];
            data.inputValues[1] = mem[addr+2];
            data.immediate = false;
            
            data.instructionLength = 3;
            data.num_inputValues = 2;

            COMMENT_CM
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
            strcpy(data.mnemonic, "CPI ");
            
            data.inputValues[0] = mem[addr+1];
            
            data.instructionLength = 2;
            data.num_inputValues = 1;

            COMMENT_CPI
            break;
        }
        case 0xFF: {
            strcpy(data.mnemonic, "RST ");
            strcpy(data.inputRegisters, "7");
            
            data.num_inputRegisters = 1;

            COMMENT_RST(7)
            break;
        }
    }
    
    return data;
}