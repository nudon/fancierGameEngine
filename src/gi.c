#include <stdlib.h>
#include <stdio.h>
#include "gi.h"
#include "attributes.h"
#include "map.h"
//temp folder for game intelligence stuff

typedef struct body_memory_struct body_memory;
typedef struct body_stats_struct body_stats;

typedef struct compound_stats_struct comp_stats;
typedef struct compound_memory_struct comp_memory;

body_stats* create_body_stats(int health, double dmg_scale, double dmg_limit);
void free_body_stats(body_stats* rm);
body_memory* make_body_memory();
void free_body_memory(body_memory* rm);

comp_stats* create_comp_stats(int jumps, int health, double dmg_scale, double dmg_limit);
void free_comp_stats(comp_stats* rm);
comp_memory* make_comp_memory();
void free_comp_memory(comp_memory* rm);

//alpha decay vec, just stores a vector and alpha decay params
#define BODY_MOVE_ALPHA 0.05
#define COMP_MOVE_ALPHA 0.05

//SMARTS
//spatial memory and reasonable thinking system
struct smarts_struct {
  compound* c;
  comp_memory* c_mem;
  comp_stats* c_stats;
  att* c_atts;

  body* b;
  body_memory* b_mem;
  body_stats* b_stats;
  att* b_atts;
};

struct body_memory_struct {
  ad_vec movement;
};

struct body_stats_struct {
  int max_health;
  int health;
  //this scales incoming damage vals
  double damage_scalar;
  //after damage scalar, take (1 - (health / max_health)) * damage limiter
  //0 < x < 1 will have affect of compound taking less damage as body gets more dmage inflicted
  // 1 < x will have compound take more damapge as body is damaged
  double damage_limiter;
};

struct compound_stats_struct {
  int max_jumps;
  int jumps_left;
  int jump_airtime;
  int max_health;
  int health;
};

struct compound_memory_struct{
  ad_vec danger;
  ad_vec helpfull;
  ad_vec movement;
};

//smarts stuff
smarts * make_smarts() {
  smarts * new = malloc(sizeof(smarts));
  new->c_mem = NULL;
  new->c_stats = NULL;
  new->c_atts = NULL;
  new->b_mem = NULL;
  new->b_stats = NULL;
  new->b_atts = NULL;
  return new;
}

void add_smarts_to_body(body* b) {
  smarts* sm = get_body_smarts(b);
  if (sm == NULL) {
    sm = make_smarts();
    set_body_smarts(b, sm);
    sm->b_stats = create_body_stats(10,1,1);
    sm->b_mem = make_body_memory();
    sm->b_atts = make_attributes();
    set_body_attribute(sm->b_atts);
  }
  sm->b = b;


  compound* c = get_owner(b);
  smarts * c_sm = get_compound_smarts(c);
  sm->c = c;
  sm->c_stats = c_sm->c_stats;
  sm->c_mem = c_sm->c_mem;
  sm->c_atts = c_sm->c_atts;
}

void add_smarts_to_comp(compound* c) {
  smarts* sm = get_compound_smarts(c);
  if (sm == NULL) {
    sm = make_smarts(); 
    set_compound_smarts(c, sm);
    sm->c_stats = create_comp_stats(1,10,1,1);
    sm->c_mem = make_comp_memory();
    sm->c_atts = make_attributes();
    set_comp_attribute(sm->c_atts);
  }
  sm->c = c;
}

//body stuff

body_stats* create_body_stats(int health, double dmg_scale, double dmg_limit) {
  body_stats* new = malloc(sizeof(body_stats));
  new->max_health = health;
  new->health = health;
  new->damage_scalar = dmg_scale;
  new->damage_limiter = dmg_limit;
  return new;
}

void free_body_stats(body_stats* rm) {
  free(rm);
}

body_memory* make_body_memory() {
  body_memory* new = malloc(sizeof(body_memory));
  init_ad_vec(&new->movement, zero_vec, BODY_MOVE_ALPHA);
  return new;
}

void free_body_memory(body_memory* rm) {
  free(rm);
}

void damage_body(body* b, double amt) {
  smarts* sm = NULL;
  body_stats* s = sm->b_stats;
  double health_scalar;
  double limiter;
  double compound_damage = 0;
  if (s != NULL) {
    amt *= s->damage_scalar;
    health_scalar = ((double)s->health / s->max_health);
    limiter = amt * s->damage_limiter;
    amt = amt * health_scalar + limiter * (1 - health_scalar);
    amt *= 0.5;
    
    if (s->health < amt) {
      compound_damage = s->health;
      s->health = 0;
      //also, potentially disable poltergeist for body
      //rather then discarding the pointer, just have an additional flag in body
    }
    else {
      compound_damage = amt;
      s->health -= amt;
    }
    //apply compound_damge to controlling compound
    compound* c = get_owner(b);
    damage_compound(c, compound_damage);
  }
}

vector_2 get_body_movement(smarts* sm) {
  return sm->b_mem->movement.vec;
}

att* get_body_attributes(smarts* sm) {
  return sm->b_atts;
}

void set_body_attributes(smarts* sm, att* atts) {
  copy_attributes(atts, sm->b_atts);
}

body* get_smarts_body(smarts* sm) {
  return sm->b;
}

//compound stuff

comp_stats* create_comp_stats(int jumps, int health, double dmg_scale, double dmg_limit) {
  comp_stats* new = malloc(sizeof(comp_stats));
  new->max_jumps = jumps;
  new->jumps_left = jumps;
  new->max_health = health;
  new->health = health;
  return new;
}

void free_comp_stats(comp_stats* rm) {
  free(rm);
}

comp_memory* make_comp_memory() {
  comp_memory* new = malloc(sizeof(comp_memory));
  init_ad_vec(&new->movement, zero_vec, COMP_MOVE_ALPHA);
  init_ad_vec(&new->helpfull, zero_vec, 0.5);
  init_ad_vec(&new->danger, zero_vec, 0.5);
  return new;
}

void free_comp_memory(comp_memory* rm) {
  free(rm);
}

void damage_compound(compound* c, double amt) {
  smarts* sm = NULL;
  comp_stats* s = sm->c_stats;
  if (s->health < amt) {
    s->health = 0;
    //do some on-death logic
  }
  else {
    s->health -= amt;
  }
}

att* get_comp_attributes(smarts* sm) {
  return sm->c_atts;
}


void set_comp_attributes(smarts* sm, att* atts) {
  copy_attributes(atts, sm->b_atts);
}

compound* get_smarts_compound(smarts* sm) {
  return sm->c;
}

///old stuff to be transfered

event* make_basic_vision_event(body* b) {
  polygon* event_area = vision_triangle(150, 200, 0);
  event* e = make_event(event_area);
  set_event(e, basic_decide_event);
  add_event_to_body(b,e);
  return e;  
}

//visual systmes

//making the cones of vision
//just make an isosolece triangle of base_width and height
//make an event out of it and attach to a body
polygon* vision_triangle(int base, int depth, double rot_off) {
  polygon* tri = createPolygon(3);
  virt_pos point = *zero_pos;
  set_base_point(tri, 0, &point);
  point = (virt_pos){.x = depth, .y = base / 2};
  virt_pos_rotate(&point, rot_off, &point);
  set_base_point(tri, 1, &point);
  point = (virt_pos){.x = depth, .y = -base / 2};
  virt_pos_rotate(&point, rot_off, &point);
  set_base_point(tri, 2, &point);
  generate_normals_for_polygon(tri);
  return tri;
}
