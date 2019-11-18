#include "myVector.h"
#include "physics.h"
#include <math.h>

#define LINEAR_DAMP 5.2
#define ROTATIONAL_DAMP .012

static int frame_count = 0;

void inc_physics_frame() {
  frame_count++;
}

vector_2* g = &(vector_2){.v1 = 0, .v2 = 0.4};

//TETHERa_BARRIER defines sort of a barrier, pushes objects away if they are closer than td
//TETHER_SPRING, defines an ideal spring situation, pushes/pulls objects
//TETHER_ROPE defines a standard rope, pulls objects together if they are farther than td

tether* default_tether = &((tether){.point_1 = NULL, .point_2 = NULL, .fizz_1 = NULL, .fizz_2 = NULL, .weight_1 = .5, .weight_2 = .5, .tether_k = .001, .tether_distance = 30, .tether_type = TETHER_SPRING});

tether* one_way_tether = &((tether){.point_1 = NULL, .point_2 = NULL, .fizz_1 = NULL, .fizz_2 = NULL, .weight_1 = 0, .weight_2 = 1, .tether_k = .45, .tether_distance = 2, .tether_type = TETHER_ROPE});

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
  fizz->moi = 10;
  fizz->velocity = *zero_vec;
  fizz->dampening = *zero_vec;
  fizz->net_acceleration = *zero_vec;
  fizz->impact = *zero_vec;
  fizz->impact_count = 0;
  fizz->tether = *zero_vec;
  fizz->gravity = (vector_2){.v1 = 0, .v2 = 0};
  fizz->rot_impact = 0;
  fizz->rot_impact_count = 0;
  fizz->rot_dampening = 0;
  fizz->rot_acceleration = 0;
  fizz->rot_velocity = 0;
  fizz->bounce = 1;
  fizz->line_damp_val = LINEAR_DAMP;
  fizz->rot_damp_val = ROTATIONAL_DAMP;
  fizz->frame_count = 0;
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

void get_avg_impact(fizzle* fizz, vector_2* result) {
  if (fizz->impact_count > 0) {
    vector_2_scale(&(fizz->impact), 1.0 / fizz->impact_count, result);
  }
  else {
    *result = *zero_vec;
  }
}

double get_avg_rot_impact(fizzle* f) {
  double ret = 0;
  if (f->rot_impact_count != 0) {
    ret = f->rot_impact / f->rot_impact_count;
  }
  return ret;
}

void set_fizzle_dampening(fizzle* fizz) {
  vector_2 vel = fizz->velocity;
  vector_2 damp = *zero_vec;
  double vel_mag = vector_2_magnitude(&vel), damp_amount = 0;
  if (!isCloseEnoughToZeroVec(&vel)) {
    make_unit_vector(&vel, &vel);
    damp_amount = vel_mag * vel_mag * fizz->line_damp_val;
    vector_2_scale(&vel, -1 * damp_amount, &damp);
    fizz->dampening = damp;
  }
  else {
    fizz->dampening = *zero_vec;
  }
}

void set_fizzle_rot_dampening(fizzle* fizz) {
  double rv = fizz->rot_velocity;
  int sign = sign_of(rv);
  fizz->rot_dampening = rv * rv * sign * -1 * fizz->rot_damp_val;
}

void calc_change(fizzle* fizz, virt_pos* t_disp, double* r_disp) {
  vector_2 prev_accel = fizz->net_acceleration;
  vector_2 new_accel = *zero_vec;
  vector_2 avg_accel = *zero_vec;

  virt_pos loc_pos = *zero_pos;
  vector_2 vel_comp = fizz->velocity;
  vector_2 acl_comp = prev_accel;
  double dt = get_dT();

  //update t_disp
  vector_2_scale(&vel_comp, dT, &vel_comp);
  vector_2_scale(&acl_comp, 0.5 * dt * dt, &acl_comp);
  vector_2_add(&vel_comp, &acl_comp, &acl_comp);
  vector_2_to_virt_pos(&acl_comp, &loc_pos);
  virt_pos_add(t_disp, &loc_pos, t_disp);
  
  //update r_disp
  double rot_delta = 0;
  rot_delta += fizz->rot_velocity * dt;
  *r_disp  += rot_delta;
 
  //update net acceleration
  if (fizz->frame_count != frame_count) {
    fizz->frame_count = frame_count;
    set_fizzle_dampening(fizz);
    get_avg_impact(fizz, &fizz->net_acceleration);
    vector_2_add(&(fizz->gravity), &fizz->net_acceleration, &fizz->net_acceleration);
    vector_2_add(&(fizz->dampening), &fizz->net_acceleration, &fizz->net_acceleration);
    vector_2_add(&(fizz->tether), &fizz->net_acceleration, &fizz->net_acceleration);
    set_impact(fizz, zero_vec);
    set_tether(fizz, zero_vec);
    fizz->impact_count = 0;
  
    new_accel = fizz->net_acceleration;
    vector_2_add(&prev_accel, &new_accel, &avg_accel);
    vector_2_scale(&avg_accel, 0.5 * dt, &avg_accel);
    vector_2_add(&avg_accel, &fizz->velocity, &fizz->velocity);

    //update rot acceleration
    set_fizzle_rot_dampening(fizz);
    fizz->rot_acceleration = get_avg_rot_impact(fizz);
    fizz->rot_acceleration += fizz->rot_dampening;

    set_rot_impact(fizz, 0);
    fizz->rot_impact_count = 0;
  
    fizz->rot_velocity += fizz->rot_acceleration * dt;
  }
}

void set_gravity(fizzle* fizz, vector_2* newGrav) {
  fizz->gravity = *newGrav;
}

void set_impact(fizzle* fizz, vector_2* newImp) {
  fizz->impact = *newImp;
}

void set_rot_impact(fizzle* f, double val) {
  f->rot_impact = 0;
}

void add_impact(fizzle* fizz, vector_2* newAdd) {
  vector_2_add(newAdd, &(fizz->impact), &(fizz->impact));
  fizz->impact_count++;
}

void add_rot_impact(fizzle* f, double val) {
  f->rot_impact += val;
  f->rot_impact_count++;
}

void set_tether(fizzle* fizz, vector_2* newTF) {
  fizz->tether = *newTF;
}

double get_mass(fizzle* f) {
  return f->mass;
}

void set_mass(fizzle* fizz, double mass) {
  fizz->mass = mass;
}

double get_moi(fizzle* f) {
  return f->moi;
}

void set_moi(fizzle* f, double n) {
  f->moi = n;
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

tether* create_tether_blank(virt_pos* p1,virt_pos* p2,fizzle* f1,fizzle* f2) {
  return create_tether(p1, p2, f1, f2, -1, -1, -1, -1, -10);
}

tether* create_tether(virt_pos* p1,virt_pos* p2,fizzle* f1,fizzle* f2,double w1,double w2,double tk, double td,int tt) {
  tether* new = malloc(sizeof(tether));
  double mag = w1 + w2;
  *new = (tether){.point_1 = p1,
		  .point_2 = p2,
		  .fizz_1 = f1,
		  .fizz_2 = f2,
		  .weight_1 = w1 / mag,
		  .weight_2 = w2 / mag,
		  .tether_k = tk,
		  .tether_distance = td,
		  .tether_type = tt};
  return new;
}

void copy_tether_params(tether* from, tether* to) {
  to->weight_1 = from->weight_1;
  to->weight_2 = from->weight_2;
  to->tether_k = from->tether_k;
  to->tether_distance = from->tether_distance;
  to->tether_type = from->tether_type;
}

void free_tether(tether* rm) {
  free(rm);
}

void set_tether_distance(tether* t, double d) {
  t->tether_distance = d;
}

void set_tether_k(tether* t, double k) {
  t->tether_k = k;
}

void get_tether_force(tether* teth, vector_2* t1, vector_2* t2) {
  double d  = distance_between_points(teth->point_1, teth->point_2);
  double len = teth->tether_distance;
  double diff = 0;
  double mag = 0;
  vector_2 t1_to_t2 = *zero_vec;
  vector_2 t2_to_t1 = *zero_vec;
  switch (teth->tether_type) {
  case TETHER_BARRIER:
    if (d < len) {
      //push away
      diff = d - len;
    }
    break;
  case TETHER_SPRING:
    if (d != len) {
      //pull/push as needed
      diff = d - len;
      
    }
    break;
  case TETHER_ROPE:
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
    vector_2 temp_1, temp_2, damp_1, damp_2;
    virt_pos_to_vector_2(teth->point_1, &temp_1);
    virt_pos_to_vector_2(teth->point_2, &temp_2);
    vector_2_sub(&temp_1, &temp_2, &t2_to_t1);
    vector_2_sub(&temp_2, &temp_1, &t1_to_t2);
    if (isZeroVec(&t2_to_t1) || isZeroVec(&t1_to_t2)) {
      t2_to_t1.v1 = 1;
      t1_to_t2.v1 = -1;
    }
    //hardcode for critically camped system, coef should eventually be a tether field
    double damp_coef = -70 * pow(teth->tether_k, 0.5);
    fizzle* f1 = teth->fizz_1;
    fizzle* f2 = teth->fizz_2;
    mag = diff * teth->tether_k;
    damp_1 = *zero_vec;
    damp_2 = *zero_vec;
    temp_1 = *zero_vec;
    temp_2 = *zero_vec;

    //spring force
    make_unit_vector(&t1_to_t2, &t1_to_t2);
    make_unit_vector(&t2_to_t1, &t2_to_t1);
    vector_2_scale(&t1_to_t2, mag, t1);
    vector_2_scale(&t2_to_t1, mag, t2);

    //dampening
    temp_1 = f1->velocity;
    temp_2 = f2->velocity;
    vector_2_scale(&temp_1, damp_coef, &damp_1);
    vector_2_scale(&temp_2, damp_coef, &damp_2);
    
    vector_2_add(t1, &damp_1, t1);
    vector_2_add(t2, &damp_2, t2);

    vector_2_scale(t1, teth->weight_1, t1);
    vector_2_scale(t2, teth->weight_2, t2);


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



void set_velocity(fizzle* f, vector_2* val) {
  f->velocity = *val;
}

void get_velocity(fizzle* f, vector_2* res) {
  *res = f->velocity;
}

void set_rot_velocity(fizzle* f, double val) {
  f->rot_velocity = val;
}

double get_rot_velocity(fizzle* f) {
  return f->rot_velocity;
}

void set_line_damp(fizzle* f, double val) {
  f->line_damp_val = val;
}

void set_rot_damp(fizzle* f, double val) {
  f->rot_damp_val = val;
}

void fizzle_velocity_diff(fizzle* source, fizzle* other, vector_2* result) {
  vector_2_sub(&other->velocity, &source->velocity, result);
}

void inv_mass_contribution(double m1, double m2, double* m1c, double* m2c) {
  mass_contribution(m1, m2, m2c, m1c);
}

void mass_contribution(double m1, double m2, double* m1c, double* m2c) {
  double sum;
  if (isinf(m1) && isinf(m2)) {
    *m1c = 0;
    *m2c = 0;
  }
  else if (isinf(m1) || isinf(m2)) {
    *m1c = 1;
    *m2c = 0;
    if (isinf(m2)) {
    *m1c = 0;
    *m2c = 1;
    }
  }
  else {
    sum = m1 + m2;
    *m1c = m1 / sum;
    *m2c = m2 / sum;
  }
}
