#include "../include/disassembler8080.h"
#include "../include/bytestream.h"

int main(int argc, char** argv) {
    FILE *ofp, *ifp;

    ifp = fopen(argv[1], "rb");
    ofp = fopen("listing.txt", "w");
    
    bytestream_t* program = bytestream_read(ifp);
    disassemble_program(*program, ofp);
    
    fclose(ofp);
    fclose(ifp);
    bytestream_destroy(program);
    
    return 0;
}