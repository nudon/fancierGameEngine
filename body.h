#ifndef FILE_MYBODY_SEEN
#define FILE_MYBODY_SEEN

struct body_struct;

#include "collider.h"
#include "physics.h"
#include "poltergeist.h"
#include "compound.h"
#include "picture.h"

typedef struct body_struct {
  struct fizzle_struct* fizz;
  struct collider_struct* coll;
  poltergeist* polt;
  struct compound_struct* owner;
  picture* pic;
  int status;
} body;

struct fizzle_struct* get_fizzle(body* body);

struct collider_struct * get_collider(body* body);

struct compound_struct* get_owner(body* body);

body* createBody(struct fizzle_struct* fizz, struct collider_struct* coll);

void free_body(body* rm);

void resolve_collisions(struct spatial_hash_map_struct* map, body* main_body);

void resolve_collision(struct spatial_hash_map_struct* map, body* body1, body* body2);

void displace_bodies(struct spatial_hash_map_struct* map, body* b1, body* b2, double mtv_mag, vector_2* b1tv_unit, vector_2* b2tv_unit);

void get_normals_of_collision(body* body1, body* body2, vector_2* mtv, vector_2* body1_norm, vector_2* body2_norm);

void inv_mass_contribution(double m1, double m2, double* m1c, double* m2c);

void mass_contribution(double m1, double m2, double* m1c, double* m2c);

struct fizzle_struct* getFizzle(body* aBody);

double getMass(body* aBody);

vector_2* getVelocity(body* aBody);

virt_pos* getCenter(body* aBody);

picture* get_picture(body* aBody);

void set_picture(body* aBody, picture* new);

#endif
