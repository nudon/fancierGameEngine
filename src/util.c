#include <stdlib.h>
#include <string.h>
#include "util.h"

void null_init_array(void** array, int lim) {
  for (int i = 0; i < lim; i++) {
    array[i] = NULL;
  }
}

int first_empty_index(char** array, int lim) {
  int ret = -1;
  for (int i = 0; i < lim; i++) {
    if (array[i] == NULL) {
      ret = i;
      i = lim;
    }
  }
  return ret;
}

int char_search(char** array, char* match, int lim) {
  int ret = -1;
  int i = 0;
  while(i < lim) {
    if (array[i] != NULL && strcmp(array[i], match) == 0) { 
      ret = i;
      i = lim;
    }
    i++;
  }
  return ret;
}
