#include "util.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief Tokenize a C string 
 * 
 * @param str - The C string to tokenize 
 * @param delim - The C string containing delimiter character(s) 
 * @param argv - A char* array that will contain the tokenized strings
 * Make sure that you allocate enough space for the array.
 */
size_t tokenize(char* str, const char* delim, char ** argv) {
  char* token;
  token = strtok(str, delim);
  size_t i = 0;
  for(; token != NULL; ++i){
    argv[i] = token;
    token = strtok(NULL, delim);
  }

  return i;
}
