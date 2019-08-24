#include "myVector.h"
#include "physics.h"
#include <math.h>

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

double get_dT_in_ms() {
  return dT;
}

double get_dT() {
  //returning in units of seconds instead of miliseconds
  return dT / 1000;
  //return 2.34;
}

void set_dT(double new) {
  dT = fmin(new,DT_LIMIT);
  //fprintf(stderr, "new dT is %f\n", dT);
}

fizzle* createFizzle() {
  fizzle* new = malloc(sizeof(fizzle));
  return new;
}

fizzle* cloneFizzle(fizzle* src) {
  fizzle* new = malloc(sizeof(fizzle));
  *new = *src;
  return new;
}

void init_fizzle(fizzle* fizz) {
  fizz->mass = 10;
  fizz->velocity = *zero_vec;
  fizz->dampening = *zero_vec;
  fizz->net_acceleration = *zero_vec;
  fizz->impact = *zero_vec;
  fizz->impact_count = 0;
  fizz->tether = *zero_vec;
  fizz->gravity = (vector_2){.v1 = 0, .v2 = 0};
  fizz->rot_acceleration = 0;
  fizz->rot_velocity = 0;
  fizz->bounce = 1;
}

void free_fizzle(fizzle* rm) {
  free(rm);
}

void add_rotational_velocity(fizzle* fizz, double delta) {
  fizz->rot_velocity += delta;
}

void add_velocity(fizzle* fizz, vector_2* delta) {
  vector_2* result = &(fizz->velocity);
  vector_2_add(delta, result, result); 
}

void update_net(fizzle* fizz) {
  //for now, just gravity.
  vector_2 loc = *zero_vec;
  vector_2_add(&(fizz->gravity), &loc, &loc);
  vector_2_add(&(fizz->dampening), &loc, &loc);
  fizz->net_acceleration = loc;
}

void get_avg_impact(fizzle* fizz, vector_2* result) {
  if (fizz->impact_count > 0) {
    vector_2_scale(&(fizz->impact), 1.0 / fizz->impact_count, result);
    //vector_2_scale(&(fizz->impact), 1.0 , result);
  }
  else {
    *result = *zero_vec;
  }
}

void update_vel(fizzle* fizz) {
  vector_2 loc = fizz->velocity;
  vector_2 net = fizz->net_acceleration;
  vector_2 temp = *zero_vec;
  vector_2_scale(&net, get_dT(), &net);
  vector_2_add(&loc, &net, &loc);
  get_avg_impact(fizz, &temp);
  vector_2_add(&temp, &loc, &loc);
  vector_2_add(&(fizz->tether), &loc, &loc);
  fizz->velocity = loc;
  fizz->rot_velocity += fizz->rot_acceleration * get_dT();
}

void update_pos_with_curr_vel(virt_pos* pos, fizzle* fizz) {
  virt_pos loc_pos = *zero_pos;
  vector_2 loc_vec = fizz->velocity;
  vector_2_scale(&loc_vec, get_dT(), &loc_vec);
  vector_2_to_virt_pos(&loc_vec, &loc_pos);
  virt_pos_add(pos, &loc_pos, pos);
}

void update_rot_with_current_vel(double* rot, fizzle* fizz) {
  *rot += fizz->rot_velocity * get_dT();
}

void set_fizzle_dampening(fizzle* fizz, double alpha) {
  //want to rework dampening at some point
  vector_2 vel = fizz->velocity;
  vector_2 damp;
  double vel_mag = vector_2_magnitude(&vel), damp_amount;
  if (!isCloseEnoughToZeroVec(&vel)) {
    make_unit_vector(&vel, &vel);
    damp_amount = vel_mag * vel_mag * alpha;
    vector_2_scale(&vel, -1 * damp_amount, &damp);
    fizz->dampening = damp;
  }
  else {
    fizz->dampening = *zero_vec;
  }
}

void set_fizzle_rot_dampening(fizzle* fizz, double alpha) {
  double rv = fizz->rot_velocity;
  fizz->rot_acceleration += rv * -1 * alpha;
}

void fizzle_update(fizzle* fizz) {
  set_fizzle_dampening(fizz, 0.003);
  set_fizzle_rot_dampening(fizz, 0.3);
  update_net(fizz);
  update_vel(fizz);
  set_impact(fizz, zero_vec);
  set_tether(fizz, zero_vec);
  fizz->rot_acceleration = 0;
  fizz->impact_count = 0;
}

void set_gravity(fizzle* fizz, vector_2* newGrav) {
  fizz->gravity = *newGrav;
}

void set_impact(fizzle* fizz, vector_2* newImp) {
  fizz->impact = *newImp;
}

void add_impact(fizzle* fizz, vector_2* newAdd) {
  vector_2_add(newAdd, &(fizz->impact), &(fizz->impact));
  fizz->impact_count++;
  
}

void set_tether(fizzle* fizz, vector_2* newTF) {
  fizz->tether = *newTF;
}

void set_mass(fizzle* fizz, double mass) {
  fizz->mass = mass;
}

double get_bounce(fizzle* f) {
  return f->bounce;
}

void set_bounce(fizzle* f, double b) {
  f->bounce = b;
}

void add_tether(fizzle* fizz, vector_2* addTF) {
  vector_2_add(addTF, &(fizz->tether), &(fizz->tether));
}

//so, tethers, they define a push/pull between 2 objects
//standard for tether type or tt
//-1 defines sort of a barrier, pushes objects away if they are closer than td
//0, defines an ideal spring situation, pushes/pulls objects
//1 defines a standard rope, pulls objects together if they are farther than td

tether* default_tether = &((tether){.point_1 = NULL, .point_2 = NULL, .fizz_1 = NULL, .fizz_2 = NULL, .weight_1 = .5, .weight_2 = .5, .tether_strength = 0, .tether_k = .001, .tether_distance = 30, .tether_type = 1});

tether* create_tether_blank(virt_pos* p1,virt_pos* p2,fizzle* f1,fizzle* f2) {
  return create_tether(p1, p2, f1, f2, -1, -1,	-1, -1, -1, -10);
}

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

void copy_tether_params(tether* from, tether* to) {
  to->weight_1 = from->weight_1;
  to->weight_2 = from->weight_2;
  to->tether_strength = from->tether_strength;
  to->tether_k = from->tether_k;
  to->tether_distance = from->tether_distance;
  to->tether_type = from->tether_type;
}

void free_tether(tether* rm) {
  free(rm);
}

void get_tether_force(tether* teth, vector_2* t1, vector_2* t2) {
  double d  = distance_between_points(teth->point_1, teth->point_2);
  double len = teth->tether_distance;
  double diff = 0;
  double mag = 0;
  double sign = 1;
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
    fprintf(stderr, "error, tether type not set\n");
    break;
  }
  if (diff != 0) {
    if (diff < 0) {
      sign = -1;
    }

    vector_2 temp_1, temp_2;
    virt_pos_to_vector_2(teth->point_1, &temp_1);
    virt_pos_to_vector_2(teth->point_2, &temp_2);
    vector_2_sub(&temp_1, &temp_2, &t2_to_t1);
    vector_2_sub(&temp_2, &temp_1, &t1_to_t2);
    make_unit_vector(&t1_to_t2, &t1_to_t2);
    make_unit_vector(&t2_to_t1, &t2_to_t1);
        
    mag = diff * teth->tether_k + sign * teth->tether_strength;

    t1_mag = mag * teth->weight_1;
    t2_mag = mag - t1_mag;
    vector_2_scale(&t2_to_t1, t1_mag, t2);
    vector_2_scale(&t1_to_t2, t2_mag, t1);
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
