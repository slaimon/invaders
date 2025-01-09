#include <stdio.h>
#include "../vstring.h"

void vstring_print(vstring_t v) {
    fwrite(v.string, sizeof(char), v.length, stdout);
}

int main(void) {
    vstring_t* vs = vstring_new(4);
    char string[100];

    for (int i = 0; i < 10; ++i) {
        sprintf(string, "%d ciao\n", i);
        vstring_concat(vs, string);
    }
    vstring_print(*vs);

    vstring_free(vs);
}