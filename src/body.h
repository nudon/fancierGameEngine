#ifndef FILE_MYBODY_SEEN
#define FILE_MYBODY_SEEN

typedef struct body_struct body;
typedef struct body_stats_struct body_stats;
typedef struct shared_input_struct shared_input;

#include "collider.h"
#include "physics.h"
#include "poltergeist.h"
#include "compound.h"
#include "graphics.h"
#include "events.h"
#include "gi.h"

#define SI_CENTER -1

shared_input* create_shared_input();
shared_input** create_shared_input_ref();
void free_shared_input(shared_input* rm);
void free_shared_input_ref(shared_input** rm);
shared_input* get_shared_input(body* b);
shared_input** get_shared_input_ref(body* b);
void set_shared_input(body* b, shared_input** si);
void un_set_shared_input(body* b);
void add_to_shared_input(virt_pos* t, double r, shared_input* si);
void get_avg_movement(shared_input* si, virt_pos* t, double* r);
void set_shared_input_origin(shared_input* si, body* b, int point);
virt_pos get_shared_input_origin(shared_input* si);
body* get_shared_input_tracking_body(shared_input* si);
virt_pos calc_rotational_offset(body* b);
virt_pos get_si_offset(body* b);

body* createBody(fizzle* fizz, struct collider_struct* coll);
body* cloneBody(body* src);
void free_body(body* rm);



int get_move_status(body* b);
void set_move_status(body* b, int val);
fizzle* get_fizzle(body* body);
fizzle* get_base_fizzle(body* b);
collider * get_collider(body* body);
compound* get_owner(body* body);
void set_owner(body* b, compound* o);
fizzle* get_fizzle(body* aBody);
//double get_mass(body* aBody);
//vector_2* get_velocity(body* aBody);
virt_pos get_body_center(body* b);
void set_body_center(body* b, virt_pos* vp);
picture* get_picture(body* aBody);
void set_poltergeist(body* b, poltergeist* polt);
poltergeist* get_poltergeist(body* b);

void set_picture(body* aBody, picture* pic);
void set_picture_by_name(body* aBody, char* fn);


void add_event_to_body(body* b, event* e);
gen_list* get_body_events(body* b);


void resolve_collisions(spatial_hash_map* map, body* main_body);
void resolve_collision(spatial_hash_map* map, body* body1, body* body2);
void inv_mass_contribution(double m1, double m2, double* m1c, double* m2c);
void mass_contribution(double m1, double m2, double* m1c, double* m2c);
void displace_bodies(spatial_hash_map* map, body* b1, body* b2, double mtv_mag, vector_2* b1tv_unit, vector_2* b2tv_unit);
void solve_for_finals(double m1, double m2, double v1i, double v2i, double* v1f, double* v2f);
void elastic_reduce(double m1, double m2, double* f1f, double* f2f, double els);
void impact(body* b1, body* b2, vector_2* normal);

tether* tether_bodies(body* b1, body* b2, tether* tether_params);

void run_body_poltergeist(body* b);

void set_body_smarts(body* b, smarts* sm);
smarts* get_body_smarts(body* b);

void set_shared_reflections(shared_input* si, int x_r, int y_r);
void push_shared_reflections(body* b);
void pull_shared_reflections(body* b);

//defined in text driver currently
int move_body(spatial_hash_map* map, body* b, virt_pos* t_disp, double r_disp);

#endif
