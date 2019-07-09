#ifndef FILE_MYBODY_SEEN
#define FILE_MYBODY_SEEN

typedef struct body_struct body;

#include "collider.h"
#include "physics.h"
#include "poltergeist.h"
#include "compound.h"
#include "picture.h"
#include "events.h"

struct body_struct {
  struct fizzle_struct* fizz;
  struct collider_struct* coll;
  poltergeist* polt;
  struct compound_struct* owner;
  picture* pic;
  int status;
  //used for holding ai related events
  gen_list* event_list;
  
};

body* createBody(fizzle* fizz, struct collider_struct* coll);
body* cloneBody(body* src);
void free_body(body* rm);



fizzle* get_fizzle(body* body);
struct collider_struct * get_collider(body* body);
compound* get_owner(body* body);
fizzle* getFizzle(body* aBody);
double getMass(body* aBody);
vector_2* getVelocity(body* aBody);
virt_pos* getCenter(body* aBody);
picture* get_picture(body* aBody);

void set_picture(body* aBody, picture* pic);
void set_picture_by_name(body* aBody, char* fn);


void add_event_to_body(body* b, event* e);
gen_list* get_body_events(body* b);


void resolve_collisions(spatial_hash_map* map, body* main_body);
void resolve_collision(spatial_hash_map* map, body* body1, body* body2);
void inv_mass_contribution(double m1, double m2, double* m1c, double* m2c);
void mass_contribution(double m1, double m2, double* m1c, double* m2c);
void displace_bodies(spatial_hash_map* map, body* b1, body* b2, double mtv_mag, vector_2* b1tv_unit, vector_2* b2tv_unit);
void get_normals_of_collision(body* body1, body* body2, vector_2* normal, vector_2* body1_norm, vector_2* body2_norm);
void solveForFinals(double m1, double m2, double v1i, double v2i, double* v1f, double* v2f);
void elasticReduce(double m1, double m2, double* f1f, double* f2f, double els);
void impact(body* b1, body* b2, vector_2* normal);

#endif
