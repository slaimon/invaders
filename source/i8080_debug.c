#include <stdlib.h>

#include "../include/i8080_debug.h"
#include "../include/i8080_disassembler.h"

#define PRINTLINE(instr, outfile) \
    fputs(i8080_instruction_toString(instr).string, outfile);


void i8080_printState (const i8080_state_t state, unsigned int numLines, FILE* ofp) {
	unsigned int i, currentPC;
	i8080_instruction_t instruction;
	
	if( ofp == NULL ) {
		fprintf(stderr, "ERROR: i8080_printState: invalid file\n");
		exit(EXIT_FAILURE);
	}
	
	fputs("\nCODE IN EXECUTION:\n\n", ofp);
	
	currentPC = state.programCounter;
	instruction = disassemble_instruction(state.mem, currentPC);
	
	fputs("-------------------------------------------\n", ofp);
	PRINTLINE(instruction, ofp)
	fputs("-------------------------------------------\n", ofp);

	currentPC += instruction.instructionLength;
	for (i = 0; i < numLines; ++i) {
		instruction = disassemble_instruction(state.mem, currentPC );
		PRINTLINE(instruction, ofp);
		currentPC += instruction.instructionLength;
	}
	
	fputs("\n\n\nREGISTERS:\n\n", ofp);
	fprintf(ofp, "[S: %01d | Z: %01d | AC: %01d | P: %01d | C: %01d]\n", state.signFlag, state.zeroFlag, state.auxCarryFlag, state.parityFlag, state.carryFlag );
	fprintf(ofp, "A:\t%02X\nB:\t%02X\nC:\t%02X\nD:\t%02X\nE:\t%02X\nH:\t%02X\nL:\t%02X\n\n", state.A, state.B, state.C, state.D, state.E, state.H, state.L);
	fprintf(ofp, "SP:\t%04X\nPC:\t%04X\n\n", state.stackPointer, state.programCounter);
	
	return;
}


int i8080_tuiDebug(i8080_state_t* state, const char* commandMapping) {
    const char defaultCommandMapping[] = "scq";
    const char* conf = (commandMapping == NULL) ? defaultCommandMapping : commandMapping;

    const char stepKey = conf[0];
    const char contKey = conf[1];
    const char quitKey = conf[2];

    const char info[] = "------------------------------------------------------\nPress (%c) to step ahead - Press (%c) to continue - Press (%c) to quit\n\n";
    char userInput;
    do {
        i8080_execute(state);
        i8080_printState(*state, 5, stdout);
        printf(info, stepKey, contKey, quitKey);
        userInput = getchar();
    } while (userInput != contKey && userInput != quitKey);


    if (userInput == contKey)
        return 0;
    else
        return 1;
}