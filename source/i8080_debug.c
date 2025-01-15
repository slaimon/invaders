#include <stdlib.h>

#include "../include/i8080_debug.h"
#include "../include/i8080_disassembler.h"

#define PRINTLINE(instr, outfile) \
    fputs(i8080_instruction_toString(instr).string, outfile);

#define TO_UPPER_CASE(c) \
	(c >= 'a' && c <= 'z') ? (c-32) : (c)

char mainSeparator[] = "_____________________________________________________________________";
char sectionSeparator[] = "-------------------------------------\n";


void i8080_printState (const i8080_t machine, unsigned int numLines, FILE* ofp) {
	unsigned int i, currentPC;
	i8080_instruction_t instruction;

	if( ofp == NULL ) {
		fprintf(stderr, "ERROR: i8080_printState: invalid file\n");
		exit(EXIT_FAILURE);
	}
	
	fputs("\nCODE IN EXECUTION:\n", ofp);
	
	currentPC = machine.programCounter;
	instruction = disassemble_instruction(machine.mem, currentPC);
	
	fputs(sectionSeparator, ofp);
	PRINTLINE(instruction, ofp)
	fputs(sectionSeparator, ofp);

	currentPC += instruction.instructionLength;
	for (i = 0; i < numLines; ++i) {
		instruction = disassemble_instruction(machine.mem, currentPC );
		PRINTLINE(instruction, ofp);
		currentPC += instruction.instructionLength;
	}
	
	fputs("\n\nREGISTERS:\n", ofp);
	fputs(sectionSeparator, ofp);
	fprintf(ofp, "[S: %01d | Z: %01d | AC: %01d | P: %01d | C: %01d]\n", machine.signFlag, machine.zeroFlag, machine.auxCarryFlag, machine.parityFlag, machine.carryFlag );
	fprintf(ofp, "A:\t%02X\nB:\t%02X\nC:\t%02X\nD:\t%02X\nE:\t%02X\nH:\t%02X\nL:\t%02X\n", machine.A, machine.B, machine.C, machine.D, machine.E, machine.H, machine.L);
	fputs(sectionSeparator, ofp);
	fprintf(ofp, "SP:\t%04X\nPC:\t%04X\n", machine.stackPointer, machine.programCounter);
	
	return;
}


int i8080_tuiDebug(i8080_t* machine, const char* customCommandMapping) {
    const char defaultCommandMapping[] = "CQ";
    const char* conf = (customCommandMapping == NULL) ? defaultCommandMapping : customCommandMapping;

	const char stepKey = '\n';
    const char contKey = TO_UPPER_CASE(conf[0]);
    const char quitKey = TO_UPPER_CASE(conf[1]);

    const char info[] = "Press (ENTER) to step ahead - Send (%c) to continue - Send (%c) to quit\n";
    char userInput;
    do {
		// step ahead
        i8080_execute(machine);

		// print machine state and commands
		puts("\n\n");
		puts(mainSeparator);
        i8080_printState(*machine, 5, stdout);
		puts(mainSeparator);
        printf(info, contKey, quitKey);

		// handle user input
		userInput = getchar();
		userInput = TO_UPPER_CASE(userInput);
    } while (userInput == stepKey);


    if (userInput == contKey)
        return 0;
    else // userInput == quitKey
        return 1;
}