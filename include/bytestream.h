#ifndef __BYTESTREAM_H__
#define __BYTESTREAM_H__

#include <stdint.h>
#include <stdio.h>

typedef struct {
	uint8_t* data;
	size_t size;
} bytestream_t;

// create a new bytestream of specified size
// returns NULL if size is zero
bytestream_t* bytestream_new(size_t size);

// create a new bytestream containing the specified string
bytestream_t* bytestream_fromString(const char* string);

// print the bytestream to a string as hex values
char* bytestream_toString(bytestream_t* stream);

// read the file and return it as a bytestream
bytestream_t* bytestream_read(FILE* ifp);

// write the given bytestream to file
// return -1 on error
int bytestream_write(bytestream_t* stream, FILE* ofp);

// free the bytestream
void bytestream_destroy(bytestream_t* stream);

#endif