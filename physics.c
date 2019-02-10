#include "myVector.h"
#include "physics.h"
#include <math.h>
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
//also need standard for dT, milliseconds should be fine
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
//cap dT at a tenth of a second
static double DT_LIMIT = 100;
double time_since_update() {
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
  //return 2.34;
}

void set_dT(double new) {
  dT = fmin(new,DT_LIMIT);
  //fprintf(stderr, "new dT is %f\n", dT);
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
  fizz->impact = *zero_vec;
  fizz->tether = *zero_vec;
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
  //nope, currently treating impact like something to add directly to velocity. 
  //vector_2_add(&(fizz->impact), &loc, &loc);
  //vector_2_add(fizz->, &loc, &loc);
  fizz->net_acceleration = loc;
}

void update_vel(fizzle* fizz) {
  vector_2 loc = fizz->velocity;
  vector_2 net = fizz->net_acceleration;
  //vector_2_scale(&net, get_dT() / 10, &net);
  vector_2_scale(&net, get_dT(), &net);
  vector_2_add(&loc, &net, &loc);
  //add impact result so final velocity is correc
  vector_2_add(&(fizz->impact), &loc, &loc);
  vector_2_add(&(fizz->tether), &loc, &loc);
  fizz->velocity = loc;
}

void update_pos_with_curr_vel(virt_pos* pos, fizzle* fizz) {
  virt_pos loc_pos = *zero_pos;
  vector_2 loc_vec = fizz->velocity;
  //vector_2_scale(&loc_vec, get_dT() / 10 , &loc_vec);
  vector_2_scale(&loc_vec, get_dT(), &loc_vec);
  vector_2_to_virt_pos(&loc_vec, &loc_pos);
  virt_pos_add(pos, &loc_pos, pos);
}

void set_fizzle_dampening(fizzle* fizz, int limit) {
  //want to rework dampening actually
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
  set_impact(fizz, zero_vec);
  set_tether(fizz, zero_vec);
}

void set_gravity(fizzle* fizz, vector_2* newGrav) {
  fizz->gravity = *newGrav;
}

void set_impact(fizzle* fizz, vector_2* newImp) {
  fizz->impact = *newImp;
}

void add_impact(fizzle* fizz, vector_2* newAdd) {
  //why did I do this
  /*
  vector_2 newImp = fizz->impact;
  vector_2_add(newAdd, &newImp, &newImp);
  fizz->impact = newImp;
  */
  
  vector_2_add(newAdd, &(fizz->impact), &(fizz->impact));
}

void set_tether(fizzle* fizz, vector_2* newTF) {
  fizz->tether = *newTF;
}

void add_tether(fizzle* fizz, vector_2* addTF) {
  vector_2_add(addTF, &(fizz->tether), &(fizz->tether));
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

tether* create_tether(virt_pos* p1,virt_pos* p2,fizzle* f1,fizzle* f2,double w1,double w2,double ts,double tk, double td,int tt) {
  tether* new = malloc(sizeof(tether));
  double mag = w1 + w2;
  *new = (tether){.point_1 = p1,
		  .point_2 = p2,
		  .fizz_1 = f1,
		  .fizz_2 = f2,
		  .weight_1 = w1 / mag,
		  .weight_2 = w2 / mag,
		  .tether_strength = ts,
		  .tether_k = tk,
		  .tether_distance = td,
		  .tether_type = tt};
  return new;
}

void free_tether(tether* rm) {
  free(rm);
}

void get_tether_force(tether* teth, vector_2* t1, vector_2* t2) {
  double d  = distance_between_points(teth->point_1, teth->point_2);
  double len = teth->tether_distance;
  double diff = 0;
  double mag = 0;
  double t1_mag = 0;
  double t2_mag = 0;
  vector_2 t1_to_t2;
  vector_2 t2_to_t1;
  
  switch (teth->tether_type) {
  case -1:
    if (d < len) {
      //push away
      diff = d - len;
    }
    break;
  case 0:
    if (d != len) {
      //pull/push as needed
      diff = d - len;
      
    }
    break;
  case 1:
    if (d > len) {
      //pull towards
      diff = d - len;
    }
    break;
  default:
    //tether probably never initialized or corrupted
    //suprised if it didn't crash when accessing points
    break;
  }
  if (diff != 0) {
    vector_2 temp_1, temp_2;
    virt_pos_to_vector_2(teth->point_1, &temp_1);
    virt_pos_to_vector_2(teth->point_2, &temp_2);
    vector_2_sub(&temp_1, &temp_2, &t2_to_t1);
    vector_2_sub(&temp_2, &temp_1, &t1_to_t2);
    make_unit_vector(&t1_to_t2, &t1_to_t2);
    make_unit_vector(&t2_to_t1, &t2_to_t1);
        
    mag = diff * teth->tether_k + teth->tether_strength;
    fprintf(stderr, "mag is %f\n", mag);
    t1_mag = mag * teth->weight_1;
    t2_mag = mag - t1_mag;
    vector_2_scale(&t2_to_t1, t1_mag, t2);
    vector_2_scale(&t1_to_t2, t2_mag, t1);
    fprintf(stderr, "t1 mag is %f, t2 mag is %f\n", t1_mag, t2_mag);
  }
}


void apply_tether(tether* teth) {
  vector_2 t1 = *zero_vec;
  vector_2 t2 = *zero_vec;
  fizzle* f1 = teth->fizz_1;
  fizzle* f2 = teth->fizz_2;
  get_tether_force(teth, &t1, &t2);
  //add t1 to f1->tether, t2 to f2->tether
  add_tether(f1, &t1);
  add_tether(f2, &t2);  
}
