#ifndef __I8080_DEBUG_H__
#define __I8080_DEBUG_H__

#include "../include/i8080.h"

// Prints information about the current machine state. The output is formatted thus:
// - First the instruction about to be executed is printed between two strokes
// - Then the following N instructions are printed, where N == numLines
// - Lastly the CPU's registers are printed
//
void i8080_printState(const i8080_state_t state, unsigned int numLines, FILE* ofp);

// Starts the TUI debugger. This tool halts the normal execution of the CPU and proceeds one step at
// a time, printing the machine's state at each step with i8080_printState. While debugging, it
// accepts a few different commands from the user:
// - STEP: proceed to the next instruction
// - CONTINUE: exit the debugger and resume normal execution
// - QUIT: exit the debugger and terminate the machine
//
// The commands are issued by pressing certain keys on the keyboard. The mapping of such keys to
// their actions can be configured via the commandMapping string. For instance, the default mapping
// of:
// - (S) for STEP
// - (C) for CONTINUE
// - (Q) for QUIT
//
// can be obtained by passing either NULL or the string "SCQ".
//
// Once the function returns, the last command issued by the user (either CONTINUE or QUIT) is
// indicated by the return value: 0 for CONTINUE, 1 for QUIT.
//
// TODO: for now, commandMapping does nothing. The default mapping is the only available one.
int i8080_tuiDebug(i8080_state_t* state, const char* commandMapping);

#endif