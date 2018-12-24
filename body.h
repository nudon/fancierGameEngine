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

void resolve_collisions(spatial_hash_map* map, body* main_body);

void resolve_collision(spatial_hash_map* map, body* body1, body* body2);

void displace_bodies(spatial_hash_map* map, body* b1, body* b2, double mtv_mag, vector_2* b1tv_unit, vector_2* b2tv_unit);

void impact_bodies(body* body1, body* body2, vector_2* b1tv_norm, vector_2* b2tv_norm);

void get_normals_of_collision(body* body1, body* body2, vector_2* mtv, vector_2* body1_norm, vector_2* body2_norm);

void get_normal_of_collision(body* body1, body* body2, vector_2* result);

double getMass(body* aBody);

vector_2* getVelocity(body* aBody);

virt_pos* getCenter(body* aBody);

#endif
