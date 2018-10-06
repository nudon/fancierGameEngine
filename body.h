#ifndef FILE_MYBODY_SEEN
#define FILE_MYBODY_SEEN

#include "collider.h"
#include "physics.h"

typedef struct body_struct {
  fizzle* fizz;
  struct collider_struct* coll;
} body;

body* createBody(fizzle* fizz, struct collider_struct* coll);

void freeBody(body* rm);

void resolve_collisions(spatial_hash_map* map, body* body);

void resolve_collision(body* body1, body* body2);

void get_normal_of_collision(body* body1, body* body2, vector_2* result);

#endif
