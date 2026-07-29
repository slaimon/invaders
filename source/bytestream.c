#include <stdlib.h>
#include <string.h> // for memset

#include "bytestream.h"
#include "safe.h"

size_t fsize(FILE* ifp) {
    int tmp = fseek(ifp, 0, SEEK_CUR);
    fseek(ifp, 0, SEEK_END);
    size_t size = ftell(ifp);

    fseek(ifp, tmp, SEEK_SET);
    return size;
}

bytestream_t* bytestream_new(size_t size) {
    if (size == 0)
        return NULL;
    
    bytestream_t* stream = safe_malloc(sizeof(bytestream_t));
    stream->data = safe_malloc(sizeof(uint8_t)*size);
    stream->size = size;
    
    memset(stream->data, 0, size);
    return stream;
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
        fprintf(stderr,"BYTESTREAM_READ WARNING: only read %lu bytes out of %lu\n", read, size);
    }

    return stream;
}

int bytestream_write(const bytestream_t* stream, FILE* ofp) {
    if (ofp == NULL) {
        fprintf(stderr,"BYTESTREAM_WRITE ERROR: invalid output file\n");
        return -1;
    }

    size_t written = fwrite(stream->data, sizeof(uint8_t), stream->size, ofp);
    if (written != stream->size) {
        fprintf(stderr,"BYTESTREAM_WRITE WARNING: only wrote %lu bytes out of %lu\n", written, stream->size);
    }

    return 0;
}

void bytestream_destroy(bytestream_t* stream) {
    free(stream->data);
    free(stream);
}
