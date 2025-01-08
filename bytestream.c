#include <stdlib.h>
#include <string.h> // for memset

#include "bytestream.h"
#include "safe.h"

bytestream_t* bytestream_new(size_t size) {
    if (size == 0)
        return NULL;
    
    bytestream_t* stream = safe_malloc(sizeof(bytestream_t));
    stream->data = safe_malloc(sizeof(uint8_t)*size);
    stream->size = size;
    
    memset(stream->data, 0, size);
    return stream;
}

bytestream_t* bytestream_fromString(const char* string) {
    size_t size = strlen(string);
    bytestream_t* stream = bytestream_new(size);
    if (stream == NULL)
        return NULL;
    
    memcpy(stream->data, string, size);
    return stream;
}

char* bytestream_toString(bytestream_t* stream) {
    size_t length = 5*stream->size;
    char* string = safe_malloc(sizeof(char)*length);
    string[length-1] = '\0';
    
    size_t pointer = 0;
    for (size_t i=0; i<stream->size-1; i++) {
        sprintf(&string[pointer], "0x%02X ", stream->data[i]);
        pointer+=5;
    }
    sprintf(&string[pointer], "0x%02X", stream->data[stream->size-1]);
    return string;
}

bytestream_t* bytestream_read(FILE* ifp) {
    if (ifp == NULL) {
        fprintf(stderr,"BYTESTREAM_READ ERROR: invalid input file\n");
        return NULL;
    }

    size_t size = safe_fsize(ifp);
    bytestream_t* stream = bytestream_new(size);

    size_t read = fread(stream->data, sizeof(uint8_t), size, ifp);
    if (read != size) {
        fprintf(stderr,"BYTESTREAM_READ WARNING: only read %llu bytes out of %llu\n", read, size);
    }

    return stream;
}

int bytestream_write(bytestream_t* stream, FILE* ofp) {
    if (ofp == NULL) {
        fprintf(stderr,"BYTESTREAM_WRITE ERROR: invalid output file\n");
        return -1;
    }

    size_t written = fwrite(stream->data, sizeof(uint8_t), stream->size, ofp);
    if (written != stream->size) {
        fprintf(stderr,"BYTESTREAM_WRITE WARNING: only wrote %llu bytes out of %llu\n", written, stream->size);
    }

    return 0;
}

void bytestream_destroy(bytestream_t* stream) {
    free(stream->data);
    free(stream);
}