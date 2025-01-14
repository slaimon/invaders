#include "../include/i8080_disassembler.h"
#include "../include/bytestream.h"
#include "../include/safe.h"

// A disassembler utility that takes two filenames as input, IN and OUT, and writes into OUT the
// disassembly of the Intel 8080 machine code found in file IN.
int main(int argc, char** argv) {
    FILE *ofp, *ifp;

    if (argc != 3) {
        printf("usage: %s INPUT_FILE OUTPUT_FILE\n", argv[0]);
        return 0;
    }

    ifp = safe_fopen(argv[1], "rb");
    ofp = safe_fopen(argv[2], "w");
    
    bytestream_t* program = bytestream_read(ifp);
    if (program == NULL) {
        printf("No valid program found\n");
        return 0;
    }
    disassemble_program(*program, ofp);
    
    fclose(ofp);
    fclose(ifp);
    bytestream_destroy(program);
    
    return 0;
}