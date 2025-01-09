#include "disassembler8080.h"
#include "bytestream.h"

int main(void) {
    FILE *ofp,*ifp;

    ifp = fopen("./INVADERS", "rb");
    ofp = fopen("./INVADERS.TXT", "w");
    
    bytestream_t* program = bytestream_read(ifp);
    disassemble_program(*program, ofp);
    
    fclose(ofp);
    fclose(ifp);
    bytestream_destroy(program);
    
    return 0;
}