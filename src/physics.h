#ifndef FILE_PHYSICS_NOTSEEN
#define FILE_PHYSICS_NOTSEEN

#include <time.h>

typedef struct fizzle_struct fizzle;
typedef struct tether_struct tether;

#include "myVector.h"
#include "collider.h"


//first, want a general physics object
//can add different types of forces
//thinking of having a net_accel,

//potential for wonky things to happen since I don't have any units
//clearest standard is to use virt_pos / dT
//assuming I want to keep this frame rate independant. getting dt, will need some other library
//also need standard for dT, milliseconds should be find



struct fizzle_struct {
  //was also thinking about having things like friction,
  //and having elascticity for collisions
  //also adding rotation velocity and accell, though not as importatn
  double mass;
  //velocity components
  vector_2 velocity;
  vector_2 impact;
  int impact_count;
  vector_2 tether;
  //acceleratoin components
  vector_2 dampening;
  vector_2 gravity;
  vector_2 net_acceleration;
  //rotational components
  double rot_acceleration;
  double rot_velocity;
  //scale for impacts
  double bounce;
};

struct tether_struct{
  virt_pos* point_1;
  virt_pos* point_2;
  fizzle* fizz_1;
  fizzle* fizz_2;
  double weight_1;
  double weight_2;
  double tether_strength;
  //behaves like a spring constant if tether is activated
  double tether_k;
  //distance of tether;
  double tether_distance;
  //defines tensor_type
  //-1, pushes points away at  < d distance
  //0, behaves as spring, trys to keep objects at d distance
  //1, behaves like a rope, pulls object in at > d distance
  int tether_type;
};

extern tether* default_tether;


fizzle* createFizzle();
fizzle* cloneFizzle(fizzle* src);
void init_fizzle(fizzle* fizz);

void free_fizzle(fizzle* rm);

tether* create_tether_blank(virt_pos* p1,virt_pos* p2,fizzle* f1,fizzle* f2);
tether* create_tether(virt_pos* p1,virt_pos* p2,fizzle* f1,fizzle* f2,double w1,double w2,double ts,double tk, double td,int tt);
void copy_tether_params(tether* from, tether* to);
void free_tether(tether* rm);

void add_rotational_velocity(fizzle* fizz, double delta);
void add_velocity(fizzle* fizz, vector_2* delta);

void set_fizzle_dampening(fizzle* fizz, double a);

void update_net(fizzle* fizz);
void get_avg_impact(fizzle* fizz, vector_2* result);
void update_vel(fizzle* fizz);
void update_pos_with_curr_vel(virt_pos* pos, fizzle* fizz);
void update_rot_with_current_vel(double* rot, fizzle* fizz);

void fizzle_update(fizzle* fizz);

void set_gravity(fizzle* fizz, vector_2* newGrav);
void set_impact(fizzle* fizz, vector_2* newImp);
void add_impact(fizzle* fizz, vector_2* newAdd);
void set_tether(fizzle* fizz, vector_2* newTF);
void set_mass(fizzle* fizz, double mass);
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


#endif
