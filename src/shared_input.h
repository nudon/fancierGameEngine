#ifndef FILE_MYSHAREDINPUT_SEEN
#define FILE_MYSHAREDINPUT_SEEN
/*
  A special system which sort of allows the ability for concave objects to be managed. In general, it "glues" together several bodies so they move and rotate (and share physics) with eachother
 */


typedef struct shared_input_struct shared_input;


#include "body.h"

shared_input* create_shared_input();

shared_input** create_shared_input_ref();

void free_shared_input_ref(shared_input** rm);

void free_shared_input(shared_input* rm);

//special logic for sharing physics along with input
//just updates total mass of shared input, and matches velocity between bodies.
void shared_input_add_fizzle(shared_input* si, fizzle* f);

//reverse actions of shared_input_add_fizzle
void shared_input_remove_fizzle(shared_input* si, fizzle* f);


void shared_input_update_rotational_offset(body* b);

//calculates rotation offset from b to si origin when b is not rotated
virt_pos shared_input_get_rotational_offset(body* b);

virt_pos shared_input_get_offset(body* b);

void shared_input_add_movement( shared_input* si, virt_pos* t, double r);


void shared_input_update_avg_movement(shared_input* si);

void shared_input_get_avg_movement(shared_input* si, virt_pos* t, double* r);


void shared_input_set_origin(shared_input* si, body* b, int point);

virt_pos shared_input_get_origin(shared_input* si);

body* shared_input_get_tracking_body(shared_input* si);

void shared_input_clear(shared_input* si);

int shared_input_get_reflection_x(shared_input* si);

int shared_input_get_reflection_y(shared_input* si);

void shared_input_set_reflections(shared_input* si, int x_r, int y_r);

#endif
