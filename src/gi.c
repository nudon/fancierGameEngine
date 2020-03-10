#include <stdlib.h>
#include <stdio.h>
#include "gi.h"
#include "attributes.h"
#include "map.h"
#include "names.h"
#include "game_state.h"
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

void set_temp_owner(shared_input* si, compound* own);
void un_set_temp_owner(shared_input* si);


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
  ad_vec look;
  ad_vec danger;
  ad_vec helpfull;
  ad_vec attack;
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

void smarts_body_init(smarts* sm) {
  sm->b_stats = create_body_stats(BODY_HEALTH,BODY_DMG_SCALE, BODY_DMG_LIMIT, BODY_DMG);
  sm->b_mem = make_body_memory();
  sm->b_atts = make_attributes();
  set_body_attribute(sm->b_atts);
}

void smarts_comp_init(smarts* sm) {
  sm->c_stats = create_comp_stats(1,10,1,1);
  sm->c_mem = make_comp_memory();
  sm->c_atts = make_attributes();
  set_comp_attribute(sm->c_atts);
}

void add_smarts_to_body(body* b) {
  smarts* sm = get_body_smarts(b);
  if (sm == NULL) {
    sm = make_smarts();
    smarts_body_init(sm);
    set_body_smarts(b, sm);

  }
  /*
  else if (sm->b_mem == NULL) {
    sm->b_mem = make_body_memory();
  }
  */
  sm->b = b;


  compound* c = get_owner(b);
  //smarts * c_sm = get_compound_smarts(c);
  sm->c = c;
  //sm->c_stats = c_sm->c_stats;
  //sm->c_mem = c_sm->c_mem;
  //sm->c_atts = c_sm->c_atts;
}

void free_body_smarts(body* b) {
  smarts* sm = get_body_smarts(b);
  if (sm == NULL) {
    return;
  }
  set_body_smarts(b, NULL);
  free_body_stats(sm->b_stats);
  free_body_memory(sm->b_mem);
  free_attributes(sm->b_atts);

}

void add_smarts_to_comp(compound* c) {
  smarts* sm = get_compound_smarts(c);
  if (sm == NULL) {
    sm = make_smarts();
    smarts_comp_init(sm);
    set_compound_smarts(c, sm);
  }
  else if (sm->c_mem == NULL) {
    sm->c_mem = make_comp_memory();
  }
  sm->c = c;
}


void free_compound_smarts(compound* c) {
  smarts* sm = get_compound_smarts(c);
  if (sm == NULL) {
    return;
  }
  set_compound_smarts(c, NULL);
  free_comp_stats(sm->c_stats);
  free_comp_memory(sm->c_mem);
  free_attributes(sm->c_atts);
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
  init_ad_vec(&new->look, zero_vec, 0.1);
  init_ad_vec(&new->movement, zero_vec, COMP_MOVE_ALPHA);
  init_ad_vec(&new->helpfull, zero_vec, 0.5);
  init_ad_vec(&new->attack, zero_vec, 0.5);
  init_ad_vec(&new->danger, zero_vec, 0.5);
  return new;
}

void free_comp_memory(comp_memory* rm) {
  free(rm);
}

void update_comp_memory(comp_memory* c_mem) {
  double dt_scale = get_dT() * FPS;
  timed_calc_ad_vec(&c_mem->look, dt_scale);
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
  copy_attributes(atts, sm->c_atts);
}

compound* get_smarts_compound(smarts* sm) {
  return sm->c;
}


void add_to_smarts(smarts* sm, char* tag, vector_2* add) {
  int b = sm->b != NULL;
  if (strcmp(tag, SM_ATTACK) == 0) {
    add_to_ad_vec(&sm->c_mem->attack, add);
  }
  else if (strcmp(tag, SM_MOVE) == 0) {
    if (b) {
      add_to_ad_vec(&sm->b_mem->movement, add);
    }
    else {
      add_to_ad_vec(&sm->c_mem->movement, add);
    }
  }
  else if (strcmp(tag, SM_LOOK) == 0) {
    add_to_ad_vec(&sm->c_mem->look, add);
  }
  else if (strcmp(tag, SM_DANGER) == 0) {
    add_to_ad_vec(&sm->c_mem->danger, add);
  }
  else if (strcmp(tag, SM_USEFULL) == 0) {
    add_to_ad_vec(&sm->c_mem->helpfull, add);
  }
}

vector_2 get_from_smarts(smarts* sm, char* tag) {
  int b = sm->b != NULL;
  if (strcmp(tag, SM_ATTACK) == 0) {
    return get_ad_vec(&sm->c_mem->attack);
  }
  else if (strcmp(tag, SM_MOVE) == 0) {
    if (b) {
      return get_ad_vec(&sm->b_mem->movement);
    }
    else {
      return get_ad_vec(&sm->c_mem->movement);
    }
  }
  else if (strcmp(tag, SM_LOOK) == 0) {
    return get_ad_vec(&sm->c_mem->look);
  }
  else if (strcmp(tag, SM_DANGER) == 0) {
    return get_ad_vec(&sm->c_mem->danger);
  }
  else if (strcmp(tag, SM_USEFULL) == 0) {
    return get_ad_vec(&sm->c_mem->helpfull);
  }
  fprintf(stderr, "error, couldn't find vector with name \"%s\"\n", tag);
  return *zero_vec;
}

void jump_action(compound* c) {
  smarts* sm = get_compound_smarts(c);
  comp_stats* c_stats = sm->c_stats;
  body* b = NULL;
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
  b = get_compound_head(c);
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
      if (strcmp(get_event_name(e), "grab") == 0 && get_shared_input(b) == NULL) {
	check_event(shm, e);
	if (get_shared_input(b) != NULL) {
	  set_temp_owner(get_shared_input(b), c);
	  done = 1;
	  printf("picked up\n");
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
  shared_input* si = NULL;
  int done = 0;
  vector_2 throw_f = *zero_vec;
  double throw_s = 6;
  fizzle* obj_f = NULL;
  while(!done && curr!=NULL) {
    b = (body*)curr->stored;
    events = get_body_events(b)->start;
    while(!done && events!=NULL) {
      e = (event*)events->stored;
      if (strcmp(get_event_name(e), "grab") == 0) {
	//so far holders are individual bodies that use other compounds si
	//removing should si will release object
	si = get_shared_input(b);
	if (si != NULL) {
	  obj_f = get_fizzle(b);
	  un_set_shared_input(b);
	  un_set_temp_owner(si);
	  get_tether(get_fizzle(b), &throw_f);
	  vector_2_scale(&throw_f, -1 * throw_s, &throw_f);
	  add_tether(obj_f, &throw_f);
	  //also need to reset holdable attributes
	  compound* c = get_smarts_compound(get_body_smarts(get_shared_input_tracking_body(si)));
	  att* atts = get_comp_attributes(get_compound_smarts(c));
	  set_holdable(atts, 1);					  
	  done = 1;
	  printf("thrown\n");
	}
      }
      events = events->next;
    }
    curr = curr->next;
  }
}

void set_temp_owner(shared_input* si, compound* own) {
  compound* c = get_owner(get_shared_input_tracking_body(si));
  gen_node* curr = get_bodies(c)->start;
  body* b = NULL;
  while(curr != NULL) {
    b = (body*)curr->stored;
    set_owner(b,own);
    curr = curr->next;
  }
}


void un_set_temp_owner(shared_input* si) {
  compound* c = get_smarts_compound(get_body_smarts(get_shared_input_tracking_body(si)));
  gen_node* curr = get_bodies(c)->start;
  body* b = NULL;
  while(curr != NULL) {
    b = (body*)curr->stored;
    set_owner(b,c);
    curr = curr->next;
  }
}

vector_2 vision_inv_distance_scale(vector_2* vec) {
  double mag = vector_2_magnitude(vec);
  double eq = 100;
  double scale = 0;
  vector_2 ret = *zero_vec;
  if (mag == 0) {
    fprintf(stderr, "warning, gave zero_vec for line of sight to inv_distance_scale\n"); 
    return ret;
  }
  if (mag < eq) {
    scale = 1;
  }
  else {
    scale = eq / mag;
  }
  scale *= eq / mag;
  vector_2_scale(vec, scale, &ret);
  return ret;
}

vector_2 vision_speed_scale(vector_2* vec, vector_2* velocity) {
  double mag = vector_2_magnitude(vec);
  double vel = vector_2_magnitude(velocity);
  double eq = 50;
  double  scale;
  vector_2 ret = *zero_vec;
  if (vel == 0) {
    fprintf(stderr, "warning, gave zero_vec for line of sight to speed_scale\n"); 
    scale = 0;
    return ret;
  }
  if (vel < eq) {
    scale = vel / eq;
  }
  else {
    scale = 1;
  }
  scale *= eq / mag;
  vector_2_scale(vec, scale, &ret);
  return ret;
}
 
 
