#ifndef __INSTRUCTION8080_H__
#define __INSTRUCTION8080_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define I8080_LITTLE_ENDIAN true

#define I8080_MNEMONIC_NAME_LENGTH  4
#define I8080_REGISTER_NAME_LENGTH  4
#define I8080_MAX_INSTRUCTION_PARAMETERS     2    // massimo numero di parametri espliciti per una istruzione (in pratica questo codice ne supporta al max. 3)
#define I8080_MAX_INSTRUCTION_LENGTH         3    // massima dimensione di una istruzione, in byte
#define I8080_MAX_FILENAME_LENGHT            20   // massima lunghezza del nome di un file

#define ADDRESS_LENGTH  6
#define OPCODE_LENGTH   3
#define MNEMONIC_LENGTH         (I8080_MNEMONIC_NAME_LENGTH + 3)
#define INPUT_REGISTERS_LENGTH  (I8080_REGISTER_NAME_LENGTH + 2)
#define ARGUMENTS_LENGTH 6

#define LINE_SIZE \
            ( ADDRESS_LENGTH + 3*OPCODE_LENGTH + MNEMONIC_LENGTH + \
              INPUT_REGISTERS_LENGTH + ARGUMENTS_LENGTH + 2 )


typedef struct {
    uint8_t opcode;
    char mnemonic[I8080_MNEMONIC_NAME_LENGTH + 1];
    uint16_t position;   // posizione nel file
    unsigned short instructionLength;    // lunghezza dell'istruzione in byte (nel nostro caso coincide con num_inputValues+1)
    
    char inputRegisters[I8080_REGISTER_NAME_LENGTH + 1];    // nome dei registri letti
    unsigned short num_inputRegisters;    // # registri letti come parametri impliciti (serve per capire se utilizzare inputRegisters)
    
    int inputValues[I8080_MAX_INSTRUCTION_PARAMETERS];    // parametri dell'operatore: positivo se valore numerico, negativo se indirizzo
    unsigned short num_inputValues;    // # parametri espliciti (serve per capire se utilizzare inputValues)
    bool immediate;
} instruction8080_t;

typedef struct {
    char string[LINE_SIZE + 1];
} i8080_line_t;

// prints one instruction to a fixed-width string
i8080_line_t instruction8080_toString(instruction8080_t instruction);

#endif