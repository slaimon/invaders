#include <stdlib.h>
#include <sys/stat.h>

#include "safe.h"
#include "bytestream.h"
#include "i8080_cpm.h"

void fail_with_msg(const char* error_msg, const char* name) {
    printf("usage: %s ROM_FILE\n", name);
    if (error_msg != NULL) {
        printf("%s\n", error_msg);
    }
    exit(EXIT_FAILURE);
}

// Takes one filename as argument and executes it like a CP/M program. The ROM is loaded at the
// correct address in memory and some supervisor calls are handled, which allows the program to
// output text to the console.
int main (int argc, char** argv) {
    if (argc != 2) {
        fail_with_msg("Exactly one argument expected.", argv[0]);
    }

    const char* path = argv[1];

    // Make sure that we're dealing with a regular file!
    struct stat st;
    stat(path, &st);
    if (!S_ISREG(st.st_mode)) {
        fail_with_msg(
            "The ROM file must be a regular file; a directory or other file type was found instead.",
            argv[0]
        );
    }

    FILE* ifp = safe_fopen(path, "rb");
    bytestream_t* program = bytestream_read(ifp);
    fclose(ifp);
    if (program == NULL) {
        fail_with_msg("No valid program found.", argv[0]);
    }

    i8080_t machine;
    i8080_init(&machine);
    i8080_memory_write(&machine, *program, 0x100);  // CP/M loads user programs at address 100h
    bytestream_destroy(program);

    machine.mem[5] = 0xC9;  // Return immediately after all CP/M calls
    machine.programCounter = 0x100;    // JMP 100h
    
    printf("Loaded ROM: %s\n", argv[1]);
    puts("--------------------------------------------------\n");
    size_t cycles = 0;
    while (true) {
        if (handle_cpm_calls(&machine))
            break;
        
        cycles += i8080_execute(&machine);
    }

    printf("Elapsed %lu cycles (%lf seconds @ 2MHz)\n\n", cycles, (double)cycles/2000000.0);
    return 0;
}