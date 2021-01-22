#ifndef FILE_GI_STRUCTS_SEEN
#define FILE_GI_STRUCTS_SEEN
/*
  Contains struct definitions, since I want to expose these to files which deal more directly with working with GI fields to determine actions, and at the same time not clog gi.c with the action determining logic
 */


#include "geometry.h"

//regen rate is in terms of stamina per second
struct stam_struct {
  int max_stamina;
  int stamina;
  double regen_rate;
};

struct body_memory_struct {
  ad_vec movement;
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

#endif
