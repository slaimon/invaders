#include <stdlib.h>
#include <string.h> // for memset

#include "../include/bytestream.h"
#include "../include/safe.h"

size_t fsize(FILE* ifp) {
    int tmp = fseek(ifp, 0, SEEK_CUR);
    fseek(ifp, 0, SEEK_END);
    int size = ftell(ifp);

    fseek(ifp, tmp, SEEK_SET);
    return size;
}

bytestream_t* bytestream_new(size_t size) {
    if (size == 0)
        return NULL;
    
    bytestream_t* stream = (bytestream_t*) safe_malloc(sizeof(bytestream_t));
    stream->data = (uint8_t*) safe_malloc(sizeof(uint8_t)*size);
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
    char* string = (char*) safe_malloc(sizeof(char)*length);
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

    size_t size = fsize(ifp);
    bytestream_t* stream = bytestream_new(size);

    size_t read = fread(stream->data, sizeof(uint8_t), size, ifp);
    if (read != size) {
        fprintf(stderr,"BYTESTREAM_READ WARNING: only read %zu bytes out of %zu\n", read, size);
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
        fprintf(stderr,"BYTESTREAM_WRITE WARNING: only wrote %zu bytes out of %zu\n", written, stream->size);
    }

    return 0;
}

void bytestream_destroy(bytestream_t* stream) {
    free(stream->data);
    free(stream);
}
