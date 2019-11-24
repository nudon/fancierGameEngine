#ifndef FILE_PHYSICS_NOTSEEN
#define FILE_PHYSICS_NOTSEEN

#include <time.h>

typedef struct fizzle_struct fizzle;
typedef struct tether_struct tether;

#include "myVector.h"
#include "collider.h"

#define TETHER_BARRIER -1
#define TETHER_ROPE 0
#define TETHER_SPRING 1
#define TETHER_CRIT_DAMP_SPRING 2

extern vector_2* g;
extern tether* default_tether;
extern tether* one_way_tether;

struct fizzle_struct {
  fizzle* anotherFizzle;
  double mass;
  //moment of inertia
  double moi;
  //linear components
  vector_2 velocity;
  vector_2 impact;
  vector_2 tether;
  vector_2 dampening;
  vector_2 gravity;
  vector_2 net_acceleration;
  //rotational components
  double rot_impact;
  double rot_dampening;
  double rot_acceleration;
  double rot_velocity;
  //impact components
  int impact_count;
  int rot_impact_count;
  double bounce;
  double line_damp_val;
  double rot_damp_val;
  int frame_count;
};

void redirect_fizzle(fizzle* a, fizzle* r);
fizzle* get_end_fizzle(fizzle* f);
void clear_other_fizzle(fizzle* f);

struct tether_struct{
  virt_pos* point_1;
  virt_pos* point_2;
  fizzle* fizz_1;
  fizzle* fizz_2;
  double weight_1;
  double weight_2;
  //behaves like a spring constant if tether is activated
  double tether_k;
  //distance of tether;
  double tether_distance;
  //defines tensor_type
  int tether_type;
};

void inc_physics_frame();

fizzle* createFizzle();
fizzle* cloneFizzle(fizzle* src);
void init_fizzle(fizzle* fizz);

void free_fizzle(fizzle* rm);

tether* create_tether_blank(virt_pos* p1,virt_pos* p2,fizzle* f1,fizzle* f2);
tether* create_tether(virt_pos* p1,virt_pos* p2,fizzle* f1,fizzle* f2,double w1,double w2,double tk, double td,int tt);
void copy_tether_params(tether* from, tether* to);
void free_tether(tether* rm);
void set_tether_distance(tether* t, double d);
void set_tether_k(tether* t, double k);

void add_rotational_velocity(fizzle* fizz, double delta);
void add_velocity(fizzle* fizz, vector_2* delta);
void add_rot_impact(fizzle* f, double val);
void set_rot_impact(fizzle* f, double val);

void set_fizzle_dampening(fizzle* fizz);
void get_avg_impact(fizzle* fizz, vector_2* result);
double get_avg_rot_impact(fizzle* f);
void calc_change(fizzle* fizz, virt_pos* t_disp, double* r_disp);

void set_gravity(fizzle* fizz, vector_2* newGrav);
void set_impact(fizzle* fizz, vector_2* newImp);
void add_impact(fizzle* fizz, vector_2* newAdd);
void set_tether(fizzle* fizz, vector_2* newTF);
void get_tether(fizzle* fizz, vector_2* res);
double get_mass(fizzle* f);
void set_mass(fizzle* fizz, double mass);
double get_moi(fizzle* f);
void set_moi(fizzle* f, double n);
double get_bounce(fizzle* f);
void set_bounce(fizzle* f, double b);

void add_tether(fizzle* fizz, vector_2* addTF);
void apply_tether(tether* teth);

//timing functions

double timeInMS();
double timespec_difference(struct timespec* t1, struct timespec* t2);

double time_since_update();

void time_update();

double get_dT();
double get_dT_in_ms();

void set_dT(double new);

void set_velocity(fizzle* f, vector_2* val);
void get_velocity(fizzle* f, vector_2* res);
void set_rot_velocity(fizzle* f, double val);
double get_rot_velocity(fizzle* f);
void set_line_damp(fizzle* f, double val);
void set_rot_damp(fizzle* f, double val);

void fizzle_velocity_diff(fizzle* source, fizzle* other, vector_2* result);

void inv_mass_contribution(double m1, double m2, double* m1c, double* m2c);
void mass_contribution(double m1, double m2, double* m1c, double* m2c);

#endif
