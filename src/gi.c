#include <stdlib.h>
#include <stdio.h>
#include "gi.h"
#include "attributes.h"
#include "map.h"
#include "names.h"
//temp folder for game intelligence stuff
typedef struct stam_struct stam;

typedef struct body_memory_struct body_memory;
typedef struct body_stats_struct body_stats;

typedef struct compound_stats_struct comp_stats;
typedef struct compound_memory_struct comp_memory;

void init_stam(stam* st);
void update_stam(stam* st);

body_stats* create_body_stats(int health, double dmg_scale, double dmg_limit, int damage);
void free_body_stats(body_stats* rm);
body_memory* make_body_memory();
void free_body_memory(body_memory* rm);
void update_body_memory(body_memory* b_mem);

comp_stats* create_comp_stats(int jumps, int health, double dmg_scale, double dmg_limit);
void free_comp_stats(comp_stats* rm);
comp_memory* make_comp_memory();
void free_comp_memory(comp_memory* rm);
void update_comp_memory(comp_memory* c_mem);

//alpha decay vec, just stores a vector and alpha decay params
#define BODY_MOVE_ALPHA 0.05
#define COMP_MOVE_ALPHA 0.05
//10, 1,1
#define BODY_HEALTH 10
#define BODY_DMG_SCALE 1
#define BODY_DMG_LIMIT 1
#define BODY_DMG 2
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

//regen rate is in terms of stamina per second
struct stam_struct {
  int max_stamina;
  int stamina;
  double regen_rate;
};

struct body_stats_struct {
  int max_health;
  int health;
  double action_cooldown;
  //this scales incoming damage vals
  double damage_scalar;
  //after damage scalar, take (1 - (health / max_health)) * damage limiter
  //0 < x < 1 will have affect of compound taking less damage as body gets more dmage inflicted
  // 1 < x will have compound take more damapge as body is damaged
  double damage_limiter;
  int contact_damage;
  stam actions;
};

struct compound_stats_struct {
  int max_jumps;
  int jumps_left;
  double max_jump_airtime;
  double jump_airtime;
  double jump_airtime_scale;
  int max_health;
  int health;
  stam actions;
};

struct compound_memory_struct{
  ad_vec danger;
  ad_vec helpfull;
  ad_vec movement;
};

//smarts stuff
smarts * make_smarts() {
  smarts * new = malloc(sizeof(smarts));
  new->c = NULL;
  new->c_mem = NULL;
  new->c_stats = NULL;
  new->c_atts = NULL;
  new->b = NULL;
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
    sm->b_stats = create_body_stats(BODY_HEALTH,BODY_DMG_SCALE, BODY_DMG_LIMIT, BODY_DMG);
    sm->b_mem = make_body_memory();
    sm->b_atts = make_attributes();
    set_body_attribute(sm->b_atts);
  }
  else if (sm->b_mem == NULL) {
    sm->b_mem = make_body_memory();
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
  else if (sm->c_mem == NULL) {
    sm->c_mem = make_comp_memory();
  }
  sm->c = c;
}

void update_smarts(smarts* sm) {
  if (sm == NULL) {
    return;
  }
  if (sm->b != NULL) {
    update_body_memory(sm->b_mem);
    update_stam(&sm->b_stats->actions);
  }
  else if (sm->c != NULL) {
    update_comp_memory(sm->c_mem);
    update_stam(&sm->c_stats->actions);
  }
}

void init_stam(stam* st) {
  st->max_stamina = 100;
  st->stamina = 0;
  st->regen_rate = 33;
}

void update_stam(stam* st) {
  double dt = get_dT();
  double t = st->stamina;
  t = t + t * dt;
  if (t > st->max_stamina) {
    st->stamina = st->max_stamina;
  }
  else {
    st->stamina = t;
  }
}

//body stuff

body_stats* create_body_stats(int health, double dmg_scale, double dmg_limit, int damage) {
  body_stats* new = malloc(sizeof(body_stats));
  new->max_health = health;
  new->health = health;
  new->damage_scalar = dmg_scale;
  new->damage_limiter = dmg_limit;
  new->contact_damage = damage;
  init_stam(&new->actions);
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

void update_body_memory(body_memory* b_mem) {
  double dt_scale = get_dT() * FPS;
  timed_calc_ad_vec(&b_mem->movement, dt_scale);
}

void contact_damage(body* b1, body* b2) {
  smarts* sm1 = get_body_smarts(b1);
  smarts* sm2 = get_body_smarts(b2);
  if (sm1 == NULL || sm2 == NULL) {
    return;
  }
  if (is_damager(sm1->b_atts) && !is_invuln(sm2->b_atts)) {
    damage_body(b2, sm1->b_stats->contact_damage);
  }
  if (is_damager(sm2->b_atts) && !is_invuln(sm1->b_atts)) {
    damage_body(b1, sm2->b_stats->contact_damage);
  }
}

void set_contact_damage(body* b, int val) {
  smarts* sm = NULL;
  if (val == -1) {
    val = BODY_DMG;
  }
  if (val > 0) {
    sm = get_body_smarts(b);
    sm->b_stats->contact_damage = val;
    if (val != 0) {
      set_damager(sm->b_atts, 1);
    }
    else {
      set_damager(sm->b_atts, 0);
    }
  }
}

void damage_body(body* b, double amt) {
  smarts* sm = get_body_smarts(b);
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
    if (s->health > 0 && s->health <= amt) {
      compound_damage = s->health;
      s->health = 0;
      fprintf(stderr, "something died!\n");
      //also, potentially disable poltergeist for body
      //rather then discarding the pointer, just have an additional flag in body
      set_picture(b, make_picture(DEAD_FN));
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
  new->max_jump_airtime = 0.33;
  new->jump_airtime = 0;
  new->jump_airtime_scale = 0.5;
  new->max_health = health;
  new->health = health;
  init_stam(&new->actions);
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

void update_comp_memory(comp_memory* c_mem) {
  double dt_scale = get_dT() * FPS;
  timed_calc_ad_vec(&c_mem->movement, dt_scale);
  timed_calc_ad_vec(&c_mem->helpfull, dt_scale);
  timed_calc_ad_vec(&c_mem->danger, dt_scale);
}

void damage_compound(compound* c, double amt) {
  smarts* sm = get_compound_smarts(c);
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

//side vision, event should prioritize fast moving things, result is rotating body towards thing
event* make_side_vision_event(body* b) {
  polygon* event_area = vision_cone(150, 110, 5, 0);
  event* e = make_event(event_area);
  //set_event(e, basic_decide_event);
  add_event_to_body(b,e);
  return e;  
}

//main vision, event should prioritize the closest thing in range, result is anaylzing thing in range and ?
event* make_main_vision_event(body* b) {
  polygon* event_area = vision_cone(220, 20, 2, 0);
  event* e = make_event(event_area);
  //set_event(e, basic_decide_event);
  add_event_to_body(b,e);
  return e;  
}

//hearing, should behave like side vision by noticing things that are moving quickly
event* make_hearing_event(body* b) {
  polygon* event_area = createNormalPolygon(9);
  set_scale(event_area, 20);
  event* e = make_event(event_area);
  //set_event(e, basic_decide_event);
  add_event_to_body(b,e);
  return e;  
}


//visual systmes

//making the cones of vision
//just make an isosolece triangle of base_width and height
//make an event out of it and attach to a body

polygon* vision_cone(int radius, double theta_deg, int steps, double rot_off) {
  double theta = theta_deg * DEG_2_RAD;
  if (theta > M_PI) {
    fprintf(stderr, "warning, theta for vision cone would be concave, capping it to a cemi-circle\n");
    theta = M_PI;
  }
  polygon* cone = createPolygon(steps);
  virt_pos point = *zero_pos;
  double theta_step = theta / steps;
  rot_off -= theta / 2.0;
  for (int i = 0; i < steps; i++) {
    point = (virt_pos){.x = radius, .y = 0};
    virt_pos_rotate(&point, i * theta_step + rot_off, &point);
    set_base_point(cone, i, &point);
  }
  generate_normals_for_polygon(cone);
  return cone;
  
}

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

void add_to_smarts_movement(smarts* sm, vector_2* add) {
  if (sm->b != NULL) {
    add_to_ad_vec(&sm->b_mem->movement, add);
  }
  else if (sm->c != NULL) {
    add_to_ad_vec(&sm->c_mem->movement, add);
  }
}

vector_2 get_smarts_movement(smarts* sm) {
  if (sm->b != NULL) {
    return get_ad_vec(&sm->b_mem->movement);
  }
  else if (sm->c != NULL) {
    return get_ad_vec(&sm->c_mem->movement);
  }
  return *zero_vec;
}

void jump_action(compound* c) {
  smarts* sm = get_compound_smarts(c);
  comp_stats* c_stats = sm->c_stats;
  body* aBody = NULL;
  fizzle* fizz = NULL;
  vector_2 jump_force = *g;
  vector_2_scale(&jump_force, 0.25, &jump_force);
  double jump_mag = 50;
  if (c_stats->jump_airtime == -1) {
    if (c_stats->jumps_left > 0) {
      c_stats->jumps_left--;
      c_stats->jump_airtime = 0;
    }
    else {
      return;
    }
  }
  jump_mag *= (1 - (c_stats->jump_airtime / c_stats->max_jump_airtime));
  //fprintf(stderr,"jumping with mag = %f, air_t = %f and max air_t = %f\n", jump_mag, c_stats->jump_airtime, c_stats->max_jump_airtime);
  //fprintf(stderr, "dt is %f\n", get_dT());
  vector_2_scale(&jump_force, -1 * jump_mag ,&jump_force);
  body* b = get_compound_head(c);
  fizz = get_fizzle(b);
  add_tether(fizz, &jump_force);
  /*
  for (gen_node* curr = get_bodies(c)->start; curr != NULL; curr = curr->next) {
    aBody = (body*)curr->stored;
    fizz = get_fizzle(aBody);
    add_tether(fizz, &jump_force);
  }
  */
  c_stats->jump_airtime += get_dT();
  if (c_stats->jump_airtime > c_stats->max_jump_airtime) {
    c_stats->jump_airtime = -1;
  }
}

void end_jump(compound* c) {
  smarts* sm = get_compound_smarts(c);
  comp_stats* c_stats = sm->c_stats;
  if (c_stats->jump_airtime != -1) {
    c_stats->jumps_left--;
    c_stats->jump_airtime = -1;
  }
}

void jump_action_reset(compound* c) {
  smarts* sm = get_compound_smarts(c);
  comp_stats* c_stats = sm->c_stats;
  c_stats->jumps_left = c_stats->max_jumps;
  c_stats->jump_airtime = -1;
}

void pickup_action(compound* c) {
  gen_node* curr = get_bodies(c)->start;
  gen_node* events;
  body* b;
  event* e;
  int done = 0;
  spatial_hash_map* shm = get_shm(get_plane_by_name(getMap(), MAIN_PLANE_NAME));
  while(!done && curr!=NULL) {
    b = (body*)curr->stored;
    events = get_body_events(b)->start;
    while(!done && events!=NULL) {
      e = (event*)events->stored;
      if (strcmp(get_event_name(e), "holder_grab") == 0) {
	check_event(shm, e);
	if (get_shared_input(b) != NULL) {
	  done = 1;
	}
      }
      events = events->next;
    }
    curr = curr->next;
  }
}

void throw_action(compound* c) {

  gen_node* curr = get_bodies(c)->start;
  gen_node* events;
  body* b;
  event* e;
  /*
  smarts* b_sm;
  att* b_atts;
  */
  shared_input* si = NULL;
  int done = 0;
  while(!done && curr!=NULL) {
    b = (body*)curr->stored;
    events = get_body_events(b)->start;
    while(!done && events!=NULL) {
      e = (event*)events->stored;
      if (strcmp(get_event_name(e), "holder_grab") == 0) {
	//need to test if something is in hand
	//so far holders are individual bodies that use other compounds si
	//removing should throw the object and not disfigure the thrower
	si = get_shared_input(b);
	if (si != NULL) {
	  un_set_shared_input(b);
	  done = 1;
	}
      }
      events = events->next;
    }
    curr = curr->next;
  }
  /*
    while(curr != NULL) {
    b = (body*)curr->stored;
    b_sm = get_body_smarts(b);
    b_atts = get_body_attributes(b_sm);
    
    curr = curr->next;
    }
  */
  printf("yeet\n");
}
