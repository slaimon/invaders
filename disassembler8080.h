#ifndef __DISASSEMBLER8080_H__
#define __DISASSEMBLER8080_H__

#include <stdio.h>
#include <stdint.h>

#define DISASSEMBLER8080_FAIL   -2  // retval on error

#define DISASSEMBLER8080_MNEMONIC_NAME_LENGTH  4
#define DISASSEMBLER8080_REGISTRY_NAME_LENGTH  3

#define DISASSEMBLER8080_MAX_INSTRUCTION_PARAMETERS     2    // massimo numero di parametri espliciti per una istruzione (in pratica questo codice ne supporta al max. 3)
#define DISASSEMBLER8080_MAX_INSTRUCTION_LENGTH         3    // massima dimensione di una istruzione, in byte
#define DISASSEMBLER8080_MAX_FILENAME_LENGHT            20   // massima lunghezza del nome di un file

#define DISASSEMBLER8080_LITTLE_ENDIAN  true


typedef struct {
    unsigned char opcode;
    char mnemonic[DISASSEMBLER8080_MNEMONIC_NAME_LENGTH+1];
    unsigned int position;   // posizione nel file
    unsigned int instructionLength;    // lunghezza dell'istruzione in byte (nel nostro caso coincide con num_inputValues+1)
    
    char inputRegisters[DISASSEMBLER8080_REGISTRY_NAME_LENGTH+1];    // nome dei registri letti
    unsigned int num_inputRegisters;    // # registri letti come parametri impliciti (serve per capire se utilizzare inputRegisters)
    
    int inputValues[DISASSEMBLER8080_MAX_INSTRUCTION_PARAMETERS];    // parametri dell'operatore: positivo se valore numerico, negativo se indirizzo
    unsigned int num_inputValues;    // # parametri espliciti (serve per capire se utilizzare inputValues)

} disassembler8080_instruction_t;

// returns a disassembly of the instruction found at the specified address in the memory 
disassembler8080_instruction_t disassemble_instruction( uint8_t* mem , unsigned int addr );

// prints the given disassembly to a file
void printline(disassembler8080_instruction_t disassembly, FILE* ofp);

#endif