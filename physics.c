#include "myVector.h"
#include "physics.h"
//first, want a general physics object
//can add different types of forces
//thinking of having a net_accel,

//potential for wonky things to happen since I don't have any units
//clearest standard is to use virt_pos / dT
//assuming I want to keep this frame rate independant. getting dt, will need some other library
//also need standard for dT, milliseconds should be find
//well, some issue with milliseconds
//having acceleration = 1 causes speed to be about 10
//need so convert to some longer time unit(or just divide getDt by some constant
//or just have my time units be in something bigger than a milisecond


//first, want a general physics object
//can add different types of forces
//thinking of having a net_accel,

//potential for wonky things to happen since I don't have any units
//clearest standard is to use virt_pos / dT
//assuming I want to keep this frame rate independant. getting dt, will need some other library
//also need standard for dT, milliseconds should be find
//for now
//also, since this is c, overflowing time will fuck me over
//can probably get by by storing a timespec,
//actually, the timespec should be good
//unless the tv_sec overloads, but it's probably a long or at least an int, so I should be fine
//#include <unistd.h>
//somewhere in main loop...
/*
  set_dT(time_between_updates());
  //do physics with dT
  //draw_frame
  time_update();


*/

double timeInMS() {
  struct timespec curr;
  clock_gettime(CLOCK_REALTIME, &curr);
  return curr.tv_sec * 1000.0 + curr.tv_nsec/1000000;
}

double timespec_difference(struct timespec* t1, struct timespec* t2) {
  return (t1->tv_sec - t2->tv_sec) * 1000.0 + (t1->tv_nsec - t2->tv_nsec) /1000000.0;
}

struct timespec ts;
double dT = 0;
double time_between_updates() {
  struct timespec tf;
  clock_gettime(CLOCK_REALTIME, &tf);
  double diff = timespec_difference(&tf, &ts);
  return diff;
}

void time_update() {
  clock_gettime(CLOCK_REALTIME, &ts);
}

double get_dT() {
  return dT;
}

void set_dT(double new) {
  dT = new;
}
//somewhere in main loop...
/*
  set_dT(time_between_updates();
  //do physics with dT
  //draw_frame
  time_update();


*/

fizzle* createFizzle() {
  fizzle* new = malloc(sizeof(fizzle));

  return new;
}

void init_fizzle(fizzle* fizz) {
  fizz->mass = 10;
  fizz->velocity = *zero_vec;
  fizz->dampening = *zero_vec;
  fizz->net_acceleration = *zero_vec;
  fizz->gravity = (vector_2){.v1 = 0, .v2 = 0};
}

void freeFizzle(fizzle* rm) {
  free(rm);
}


/*
void add_acceleration(fizzle* fizz, vector_2* delta) {
  vector_2* result = &(fizz->input);
  vector_2_add(delta, result, result); 
}
*/


void add_velocity(fizzle* fizz, vector_2* delta) {
  vector_2* result = &(fizz->velocity);
  vector_2_add(delta, result, result); 
}

void update_net(fizzle* fizz) {
  //for now, just gravity.
  vector_2 loc = *zero_vec;
  vector_2_add(&(fizz->gravity), &loc, &loc);
  vector_2_add(&(fizz->dampening), &loc, &loc);
  //vector_2_add(fizz->, &loc, &loc);
  fizz->net_acceleration = loc;
}

void update_vel(fizzle* fizz) {
  vector_2 loc = fizz->velocity;
  vector_2 net = fizz->net_acceleration;
  vector_2_scale(&net, get_dT(), &net);
  vector_2_add(&loc, &net, &loc);
  fizz->velocity = loc;
}

void update_pos_with_curr_vel(virt_pos* pos, fizzle* fizz) {
  virt_pos loc_pos;
  vector_2 loc_vec = fizz->velocity;
  vector_2_scale(&loc_vec, get_dT(), &loc_vec);
  vector_2_to_virt_pos(&loc_vec, &loc_pos);
  virt_pos_add(pos, &loc_pos, pos);
}

void set_fizzle_dampening(fizzle* fizz, int limit) {
  //have sign of scaling be same as limit - velocity
  vector_2 vel = fizz->velocity;
  vector_2 damp;
  double vel_mag = vector_2_magnitude(&vel), damp_amount;
  if (!isCloseEnoughToZeroVec(&vel)) {
    make_unit_vector(&vel, &vel);
    damp_amount = vel_mag / limit;
    vector_2_scale(&vel, -1 * damp_amount, &damp);
    fizz->dampening = damp;
  }
  else {
    fizz->dampening = *zero_vec;
  }
}

void fizzle_update(fizzle* fizz) {
  set_fizzle_dampening(fizz, 100);
  update_net(fizz);
  update_vel(fizz);
}

//generally, somehow associating a polygon and fizzle
/*
  update_net(fizz);
  update_vel(fizz);
  virt_pos old, new; //INIT TO ZEROS
  old = poly->center;
  new = poly->center;
  update_pos_with_curr_vel(&new, fizz);
  if (safe_update(fuck I need more things
  keept track of old and new positionss before this
  just mindlessly update polygon here

  update_net(fizz);
  update_vel(fizz);
  update_pos_with_curr_vel(poly->center, fizz);
*/

//so, if I don't care about collisions, then this is fine
//issue is colliding. generally I'll need more information about collision then the fact that one happens
//generally I will probably be find with a normal vector, but potentially other things as well
//probably better to actually resolve things like that between fizzles, though I'd still probably need a normal force for some things

//created body file to server this purpose


