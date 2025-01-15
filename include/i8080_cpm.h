#ifndef __I8080_CPM_H__
#define __I8080_CPM_H__

#include "i8080.h"

// returns true iff the emulator should halt
bool handle_cpm_calls(i8080_t* machine, FILE* ofp);

#define handle_cpm_calls(x) \
    handle_cpm_calls(x, stdout)

#endif