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

    i8080_state_t state;
    i8080_init(&state);
    int retval = i8080_tuiDebug(&state, NULL);

    printf("user requested %s\n", (retval == 0) ? "CONTINUE" : "QUIT");
    return 0;
}