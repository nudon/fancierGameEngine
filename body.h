#ifndef FILE_MYBODY_SEEN
#define FILE_MYBODY_SEEN

#include "collider.h"
#include "physics.h"
#include "poltergeist.h"

typedef struct body_struct {
  fizzle* fizz;
  struct collider_struct* coll;
  struct poltergeist_struct* polt;
  //status is kind of a weird flag I've been using
  //so far it's been used as a "this object needs to be updated" flag
  int status;
} body;

body* createBody(fizzle* fizz, struct collider_struct* coll);

void freeBody(body* rm);

void resolve_collisions(spatial_hash_map* map, body* body);

void resolve_collisions(spatial_hash_map* map, body* main_body);

void displace_bodies(spatial_hash_map* map, body* b1, body* b2, vector_2* norm);

void get_normal_of_collision(body* body1, body* body2, vector_2* result);

#endif
