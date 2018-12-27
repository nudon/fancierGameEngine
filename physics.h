#ifndef FILE_PHYSICS_NOTSEEN
#define FILE_PHYSICS_NOTSEEN

#include "myVector.h"
#include "collider.h"


//first, want a general physics object
//can add different types of forces
//thinking of having a net_accel,

//potential for wonky things to happen since I don't have any units
//clearest standard is to use virt_pos / dT
//assuming I want to keep this frame rate independant. getting dt, will need some other library
//also need standard for dT, milliseconds should be find



typedef
struct {
  //was also thinking about having things like friction,
  //and having elascticity for collisions
  //also adding rotation velocity and accell, though not as importatn
  double mass;
  vector_2 velocity;
  //acceleratoin components
  //vector_2 input; ? relic of force-input days, now vel_input is a thing
  vector_2 dampening;
  //for storing resulting velocities from impact of collision
  vector_2 impact;
  vector_2 gravity;
  vector_2 net_acceleration;
} fizzle;


fizzle* createFizzle();

void init_fizzle(fizzle* fizz);

void freeFizzle(fizzle* rm);

//void add_acceleration(fizzle* fizz, vector_2* delta);

void add_velocity(fizzle* fizz, vector_2* delta);

void set_fizzle_dampening(fizzle* fizz, int limit);

void update_net(fizzle* fizz);

void update_vel(fizzle* fizz);

void update_pos_with_curr_vel(virt_pos* pos, fizzle* fizz);

void fizzle_update(fizzle* fizz);

void set_gravity(fizzle* fizz, vector_2* newGrav);

//void resolve_collisions();

//void resolve_collision(struct collider_struct* coll1, struct collider_struct* coll2);



//my psuedo time library
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
#include <time.h>

double timeInMS();

double timespec_difference(struct timespec* t1, struct timespec* t2);

double time_since_update();

void time_update();

double get_dT();

void set_dT(double new);

#endif
