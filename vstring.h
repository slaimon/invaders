#ifndef __VSTRING_H__
#define __VSTRING_H__

#include <sys/un.h>

/*
   A vstring is a string that can grow in length adaptively
   like a vector and supports concatenation.
*/

typedef struct {
   char* v;
   size_t length;
   size_t maxsize;
} vstring_t;

// crea una vstring specificando la lunghezza iniziale
vstring_t* vstring_new(size_t length);

// leggi la locazione di memoria i, se i < length
// termina il processo se i >= length
char vstring_get(vstring_t* v, size_t i);

// scrivi nella locazione di memoria i, se i < length
// termina il processo se i >= length
void vstring_set(vstring_t* v, size_t i, char element);

// aggiungi un elemento in fondo alla vstring (incrementa length)
void vstring_append(vstring_t* v, char element);

// aggiungi una stringa normale in fondo alla vstring
void vstring_concat(vstring_t* v, char* string);

// libera il vettore
void vstring_free(vstring_t* v);

#endif
