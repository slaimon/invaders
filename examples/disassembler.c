#include "../include/i8080_disassembler.h"
#include "../include/bytestream.h"
#include "../include/safe.h"

// A utility program that prints out a disassembly of the Intel 8080 machine code found in the input file.
int main(int argc, char** argv) {
    FILE *ifp;

    if (argc != 2) {
        printf("usage: %s <input_file>\n", argv[0]);
        printf("prints out a disassembly of the i8080 machine code found in the input file.\n");
        return 0;
    }

    ifp = safe_fopen(argv[1], "rb");
    
    bytestream_t* program = bytestream_read(ifp);
    if (program == NULL) {
        printf("No valid program found\n");
        return 0;
    }
    disassemble_program(*program, stdout);
    
    fclose(ifp);
    bytestream_destroy(program);
    
    return 0;
}