#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "flags.h"

#define COMP_PREY 0 
#define COMP_HUNTER 1
#define COMP_USER 2
#define COMP_TRAVEL 3
#define COMP_HOLDABLE 4

#define BODY_INVULN 0
#define BODY_DAMAGER 1

#define DRAW_OUTLINE 0
#define DRAW_PICTURE 1
#define DRAW_EVENTS 2
#define DRAW_BBOX 3


#define NUM_BITS 8
typedef uint8_t flag_bits;

int test_bit(flag_bits a, int bit_place);

void set_bit(flag_bits *a, int bit_place, int val);

enum flag_type {compound_flags, body_flags, draw_flags, not_set};

struct flags_struct {
  enum flag_type mode;
  flag_bits flag_bits;
};

flags* make_flags() {
  flags* new = calloc(1,sizeof(flags));
  new->mode = not_set;
  return new;
}

void flags_set_type_body(flags* a) {
  a->mode = body_flags;
}

void flags_set_type_comp(flags* a) {
  a->mode = compound_flags;
}

void flags_set_type_draw(flags* a) {
  a->mode = draw_flags;
}

void free_flags(flags* rm) {
  free(rm);
}

int test_bit(flag_bits a, int bit_place) {
  return (a >> bit_place) % 2;
}

//have to set bit_place in a to zero first to be able to use | to set it
void set_bit(flag_bits *a, int bit_place, int val) {
  val = val % 2;
  int bf = ~(1 << bit_place);
  int bit = val << bit_place;
  int temp = *a & bf;
  *a = temp | bit;  
}

int is_invuln(flags* a) {
  return test_bit(a->flag_bits, BODY_INVULN);
}

void set_invuln(flags *a, int val) {
  set_bit(&(a->flag_bits), BODY_INVULN, val);
}

int is_damager(flags* a) {
  return test_bit(a->flag_bits, BODY_DAMAGER);
}

void set_damager(flags *a, int val) {
  set_bit(&(a->flag_bits), BODY_DAMAGER, val);
}

int is_prey(flags *a) {
  return test_bit(a->flag_bits, COMP_PREY);
}
void set_prey(flags *a, int val) {
  set_bit(&(a->flag_bits), COMP_PREY, val);
}

int is_hunter(flags *a) {
  return test_bit(a->flag_bits, COMP_HUNTER);
}
void set_hunter(flags *a, int val) {
  set_bit(&(a->flag_bits), COMP_HUNTER, val);
}

int is_user(flags *a) {
  return test_bit(a->flag_bits, COMP_USER);
}
void set_user(flags *a, int val) {
  set_bit(&(a->flag_bits), COMP_USER, val);
}

int is_travel(flags *a) {
  return test_bit(a->flag_bits, COMP_TRAVEL);
}
void set_travel(flags *a, int val) {
  set_bit(&(a->flag_bits), COMP_TRAVEL, val);
}

int is_holdable(flags *a) {
  return test_bit(a->flag_bits, COMP_HOLDABLE);
}
void set_holdable(flags *a, int val) {
  set_bit(&(a->flag_bits), COMP_HOLDABLE, val);
}

int is_draw_outline(flags* a) {
  return test_bit(a->flag_bits, DRAW_OUTLINE);
}

void set_draw_outline(flags* a, int val) {
  set_bit(&(a->flag_bits), DRAW_OUTLINE, val);
}

int is_draw_picture(flags* a) {
  return test_bit(a->flag_bits, DRAW_PICTURE);
}

void set_draw_picture(flags* a, int val) {
  set_bit(&(a->flag_bits), DRAW_PICTURE, val);
}

int is_draw_events(flags* a) {
  return test_bit(a->flag_bits, DRAW_EVENTS);
}

void set_draw_events(flags* a, int val) {
  set_bit(&(a->flag_bits), DRAW_EVENTS, val);
}

int is_draw_bbox(flags* a) {
  return test_bit(a->flag_bits, DRAW_BBOX);
}

void set_draw_bbox(flags* a, int val) {
  set_bit(&(a->flag_bits), DRAW_BBOX, val);
}

void copy_flags(flags* src, flags* dst) {
  if (src->mode != dst->mode) {
    fprintf(stderr, "warning, flag types not equal\n");
  }
  dst->flag_bits = dst->flag_bits;
}


char* flags_to_text(flags* flags) {
  char* text = malloc(sizeof(char*) * (NUM_BITS + 1)) ;
  snprintf(text, NUM_BITS, "%d", flags->flag_bits);
  return text;
}

flags* text_to_flags(char* text) {
  long val = atoi(text);
  if (val > pow(2, NUM_BITS) - 1) {
    fprintf(stderr, "Warning, bit val is larger than flags data type can hold\n");
  }
  flag_bits temp = (flag_bits)val;
  flags* flags = make_flags();
  flags->flag_bits = temp;
  return flags;
}
