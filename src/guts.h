#ifndef FILE_MYGUTS_SEEN
#define FILE_MYGUTS_SEEN
/*
  holds some logic that deals more with engine internals rather than struct management, and since they are sort of a critical nerve center of the game they are kept in the same file, sort of like a more beefier util library.
 */
#include "body.h"
#include "collider.h"

//finds and fixes all collisions for body
void resolve_collisions(spatial_hash_map* map, body* main_body);
//resolves a single pair of bodies that collide
void resolve_collision(spatial_hash_map* map, body* body1, body* body2);
//preliminarily displaces away from eachother to resolve collision
void displace_bodies(spatial_hash_map* map, body* b1, body* b2, double mtv_mag, vector_2* b1tv_unit, vector_2* b2tv_unit);
//computes final velocities for objects
void solve_for_finals(double m1, double m2, double v1i, double v2i, double* v1f, double* v2f);
//may adjust final velocity somehow
void elastic_reduce(double m1, double m2, double* f1f, double* f2f, double els);
//adds additional forces to bodies 
void impact(body* b1, body* b2, vector_2* normal);
void impact_torque(body* b1, body* b2, vector_2* b1_norm, vector_2* b2_norm, virt_pos* poc);

//standard load_event poly
event* make_load_event(virt_pos* cent);
//creates a load zone and inserts it into plane
void insert_load_zone_into_plane(char* from_map, char* to_map, plane* from_plane, char* to_plane_name, virt_pos* from_pos, virt_pos* to_pos);

//sets builder/user staus
void make_compound_user(compound* comp);
void make_compound_builder(compound* comp);

//special actions

//jumping logic for while jump button is held, when it is released, and when a body touches the ground again
void jump_action(compound* c);
void end_jump(compound* c);
void jump_action_reset(compound* c);

//pickup and throwing
//pickup involves searching through body parts and events, looking for a grab event, then checking event in shm map, running event logic, then setting temp owner in whatever was picked up
//the shm is just from main plane in current map, which is not going to work always 
void pickup_action(compound* c);
void throw_action(compound* c);

//temporarily sets owner of things in si to be own, and restores original owner later
//relies on most logic using get_owners to find compound, but another way is to get the compound stored in body_smarts
//temporarily setting owners keeps picked up things from hurting the thing that picked it up
void set_temp_owner(shared_input* si, compound* own);
void un_set_temp_owner(shared_input* si);

//util function for vision

//modifies vectors based for use in vision, so cover some things like closer objects should have a higher "weight" in decisions than far away, and faster things should also draw more attention
vector_2 vision_inv_distance_scale(vector_2* vec);
vector_2 vision_speed_scale(vector_2* vec, vector_2* velocity);

//special poltergeists

void user_poltergeist(body* user_body, vector_2* t_disp, double* r_disp);
void builder_poltergeist(body* builder, vector_2* t_disp, double* r_disp);
void basic_brain(body* b, vector_2* t_disp, double* r_disp);
void standard_poltergeist(body* body, vector_2* t_disp, double* r_disp);
void look_poltergeist(body* body, vector_2* t_disp, double* r_disp);

//takes body, applys a constant force in direction of torsos velocity
//paired with a tether between body and torso, holds body in front of torso
void holder_poltergeist(body* b, vector_2* t_disp, double* r_disp);


//special events
event* make_basic_vision_event(body* b);

//side vision, event should prioritize fast moving things, result is rotating body towards thing
event* make_side_vision_event(body* b);

//main vision, event should prioritize the closest thing in range, result is anaylzing thing in range and ?
event* make_main_vision_event(body* b);

//hearing, should behave like side vision by noticing things that are moving quickly
event* make_hearing_event(body* b);

event* make_foot_step_event(body* b);

event* make_grab_event(body* b);

event* make_builder_select_event(body* b);


void side_sight_event(event* e, body* b2, virt_pos* poc);

void main_sight_event(event* e, body* b2, virt_pos* poc);

void foot_placement(event* e, body* trigger, virt_pos* poc);

void foot_step(event* e, body* trigger, virt_pos* poc);

void grab_event(event* e, body* trigger, virt_pos* poc);

void builder_select_event(event* e, body* b2, virt_pos* poc);

#endif
