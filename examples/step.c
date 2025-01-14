#include "../include/safe.h"
#include "../include/bytestream.h"
#include "../include/i8080_debug.h"

// A program that takes one filename as input, IN, and begins execution of the Intel 8080 machine
// code found therein. The execution proceeds one step at a time, and after each step a crude TUI
// (terminal user interface) is printed showing the contents of the CPU registers and a disassembly
// of the instructions that will be executed next.
//
// The program quits when the user types either "Q" or "C" (case insensitive) and presses enter.
int main(int argc, char** argv) {
    if (argc != 2) {
        printf("usage: %s ROM_FILE\n", argv[0]);
    }

    FILE* ifp = safe_fopen(argv[1], "rb");
    bytestream_t* program = bytestream_read(ifp);
    if (program == NULL) {
        printf("No valid program found\n");
        return 0;
    }
    fclose(ifp);

    i8080_t machine;
    i8080_init(&machine);
    i8080_memory_write(&machine, *program, 0);
    bytestream_destroy(program);

    int retval = i8080_tuiDebug(&machine, NULL);

    printf("user requested %s\n", (retval == 0) ? "CONTINUE" : "QUIT");
    return 0;
}