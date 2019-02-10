#ifndef FILE_COMPOUND_SEEN
#define FILE_COMPOUNT_SEEN
#include "myList.h"
//okay, compound time
//will probably need a compound pointer in bodies as well
typedef struct compound_struct {
  //list of bodies in compound
  gen_list* bp;
} compound;

#endif
