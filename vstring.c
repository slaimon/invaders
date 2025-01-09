#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "safe.h"
#include "vstring.h"

vstring_t* vstring_new(size_t length) {
   vstring_t* v = safe_malloc(sizeof(vstring_t));
   
   v->length = 0;
   v->maxsize = length + length/2;
   v->string = safe_malloc(sizeof(char) * v->maxsize);
   
   return v;
}

char vstring_get(vstring_t* v, size_t i) {
   if (i >= v->length) {
      fprintf(stderr, "vstring error: invalid input for get\n");
      exit(EXIT_FAILURE);
   }
   
   return v->string[i];
}

void vstring_set(vstring_t* v, size_t i, char element) {
   if (i >= v->length) {
      fprintf(stderr, "vstring error: invalid input for set\n");
      exit(EXIT_FAILURE);
   }
   
   v->string[i] = element;
}

void vstring_append(vstring_t* v, char element) {
   if (v->length == v->maxsize) {
      // se il vettore deve essere riallocato, la sua maxsize viene incrementata di 1.5 volte
      // (fattore di crescita = 1.5)
      v->maxsize += v->maxsize/2;
      v->string = safe_realloc(v->string, sizeof(char), v->maxsize);
   }
   
   v->string[v->length] = element;
   v->length++;
   return;
}

void vstring_concat(vstring_t* v, char* string) {
   size_t str_length = strlen(string);
   size_t new_length = v->length + str_length;

   if (new_length >= v->maxsize) {
      v->maxsize = new_length + new_length/2;
      v->string = safe_realloc(v->string, sizeof(char), v->maxsize);
   }

   memcpy(&v->string[v->length], string, str_length);
   v->length = new_length;
   return;
}

void vstring_free(vstring_t* v) {
   free(v->string);
   free(v);
}
