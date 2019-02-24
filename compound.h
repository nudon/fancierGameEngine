#ifndef FILE_COMPOUND_SEEN
#define FILE_COMPOUND_SEEN

struct compound_struct;

#include "body.h"
#include "myList.h"
//okay, compound time
//will probably need a compound pointer in bodies as well
typedef struct compound_struct {
  //list of bodies in compound
  gen_list* bp;
} compound;

compound* create_compound();

void add_body_to_compound(compound* comp, struct body_struct* body);

void tether_join_compound(compound* comp, struct tether_struct* teth, gen_list* append);

#endif
