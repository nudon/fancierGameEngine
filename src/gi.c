#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "gi.h"
#include "gi_structs.h"
#include "flags.h"
#include "map.h"
#include "names.h"
#include "game_state.h"


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
#define BODY_HEALTH 10
#define BODY_DMG_SCALE 1
#define BODY_DMG_LIMIT 1
#define BODY_DMG 2

//spatial memory and reasonable thinking system
struct smarts_struct {
  compound* c;
  comp_memory* c_mem;
  comp_stats* c_stats;
  flags* c_flags;

  body* b;
  body_memory* b_mem;
  body_stats* b_stats;
  flags* b_flags;
};

smarts * make_smarts() {
  smarts * new = malloc(sizeof(smarts));
  new->c = NULL;
  new->c_mem = NULL;
  new->c_stats = NULL;
  new->c_flags = NULL;
  new->b = NULL;
  new->b_mem = NULL;
  new->b_stats = NULL;
  new->b_flags = NULL;
  return new;
}

void smarts_body_init(smarts* sm) {
  sm->b_stats = create_body_stats(BODY_HEALTH,BODY_DMG_SCALE, BODY_DMG_LIMIT, BODY_DMG);
  sm->b_mem = make_body_memory();
  sm->b_flags = make_flags();
  flags_set_type_body(sm->b_flags);
}

void smarts_comp_init(smarts* sm) {
  sm->c_stats = create_comp_stats(1,10,1,1);
  sm->c_mem = make_comp_memory();
  sm->c_flags = make_flags();
  flags_set_type_comp(sm->c_flags);
}

void add_smarts_to_body(body* b) {
  smarts* sm = get_body_smarts(b);
  if (sm == NULL) {
    sm = make_smarts();
    smarts_body_init(sm);
    set_body_smarts(b, sm);

  }
  sm->b = b;


  compound* c = get_owner(b);
  sm->c = c;
}

void free_body_smarts(body* b) {
  smarts* sm = get_body_smarts(b);
  if (sm == NULL) {
    return;
  }
  set_body_smarts(b, NULL);
  free_body_stats(sm->b_stats);
  free_body_memory(sm->b_mem);
  free_flags(sm->b_flags);

}

void add_smarts_to_comp(compound* c) {
  smarts* sm = get_compound_smarts(c);
  if (sm == NULL) {
    sm = make_smarts();
    smarts_comp_init(sm);
    set_compound_smarts(c, sm);
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
  free_flags(sm->c_flags);
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
  double t = fmin(st->stamina + st->regen_rate * dt, st->max_stamina);
  st->stamina = t;
}


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
  if (is_damager(sm1->b_flags) && !is_invuln(sm2->b_flags)) {
    damage_body(b2, sm1->b_stats->contact_damage);
  }
  if (is_damager(sm2->b_flags) && !is_invuln(sm1->b_flags)) {
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
      set_damager(sm->b_flags, 1);
    }
    else {
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
      fprintf(stderr, "some part died!\n");
      set_picture(b, make_picture(DEAD_FN));
    }
    else {
      compound_damage = amt;
      s->health -= amt;
    }
    compound* c = get_owner(b);
    damage_compound(c, compound_damage);
  }
}

vector_2 get_body_movement(smarts* sm) {
  return sm->b_mem->movement.vec;
}

flags* get_body_flags(smarts* sm) {
  return sm->b_flags;
}

void set_body_flags(smarts* sm, flags* f) {
  copy_flags(f, sm->b_flags);
}

body* get_smarts_body(smarts* sm) {
  return sm->b;
}

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
    fprintf(stderr, "something died!\n");
    //do some on-death logic
  }
  else {
    s->health -= amt;
  }
}

flags* get_comp_flags(smarts* sm) {
  return sm->c_flags;
}

void set_comp_flags(smarts* sm, flags* f) {
  copy_flags(f, sm->c_flags);
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

comp_stats* smarts_get_comp_stats(smarts* sm) {
  if (sm->c != NULL &&  sm->c_stats != NULL) {
    return sm->c_stats;
  }
  return NULL;
}
body_stats* smarts_get_body_stats(smarts* sm) {
  if (sm->b != NULL &&  sm->b_stats != NULL) {
    return sm->b_stats;
  }
  return NULL;
}
