#include "../include/safe.h"
#include "../include/bytestream.h"
#include "../include/i8080_debug.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("usage: %s INPUT_FILE\n", argv[0]);
    }

    FILE* ifp = safe_fopen(argv[1], "rb");
    bytestream_t* program = bytestream_read(ifp);
    if (program == NULL) {
        printf("No valid program found\n");
        return 0;
    }
    fclose(ifp);

    i8080_state_t state;
    i8080_init(&state);
    i8080_setMemory(&state, *program, 0);
    bytestream_destroy(program);

    int retval = i8080_tuiDebug(&state, NULL);

    printf("user requested %s\n", (retval == 0) ? "CONTINUE" : "QUIT");
    return 0;
}