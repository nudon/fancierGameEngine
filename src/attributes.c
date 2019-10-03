#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "attributes.h"


//pair bits, prey run from hunters
//should probably make hunters chase prey
#define PREY_BIT 0
#define HUNTER_BIT 1
//designated to mark the user so user only events like camera/map transitions happen
#define USER_BIT 2
//travel bit because I probably don't want things like walls to traverse maps
#define TRAVEL_BIT 3

#define NUM_BITS 8
typedef uint8_t att_bits;

int test_bit(att_bits a, int bit_place);

void set_bit(att_bits *a, int bit_place, int val);

enum att_type {compound_atts, body_atts, not_set};

struct att_struct {
  enum att_type mode;
  att_bits flag_atts;
};

att* make_attributes() {
  att* new = calloc(1,sizeof(att));
  new->mode = not_set;
  return new;
}

void set_body_attribute(att* a) {
  a->mode = body_atts;
}

void set_comp_attribute(att* a) {
  a->mode = compound_atts;
}

void free_attributes(att* rm) {
  free(rm);
}

int test_bit(att_bits a, int bit_place) {
  return (a >> bit_place) % 2;
}

void set_bit(att_bits *a, int bit_place, int val) {
  val = val % 2;
  int bf = ~(1 << bit_place);
  int bit = val << bit_place;
  int temp = *a & bf;
  *a = temp | bit;  
}

int is_prey(att *a) {
  return test_bit(a->flag_atts, PREY_BIT);
}
void set_prey(att *a, int val) {
  set_bit(&(a->flag_atts), PREY_BIT, val);
}

int is_hunter(att *a) {
  return test_bit(a->flag_atts, HUNTER_BIT);
}
void set_hunter(att *a, int val) {
  set_bit(&(a->flag_atts), HUNTER_BIT, val);
}

int is_user(att *a) {
  return test_bit(a->flag_atts, USER_BIT);
}
void set_user(att *a, int val) {
  set_bit(&(a->flag_atts), USER_BIT, val);
}

int is_travel(att *a) {
  return test_bit(a->flag_atts, TRAVEL_BIT);
}
void set_travel(att *a, int val) {
  set_bit(&(a->flag_atts), TRAVEL_BIT, val);
}

void copy_attributes(att* src, att* dst) {
  if (src->mode != dst->mode) {
    fprintf(stderr, "warning, attribute type not set\n");
  }
  dst->flag_atts = dst->flag_atts;
}


char* atts_to_text(att* att) {
  char* text = malloc(sizeof(char*) * (NUM_BITS + 1)) ;
  snprintf(text, NUM_BITS, "%d", att->flag_atts);
  return text;
}

att* text_to_atts(char* text) {
  long val = atoi(text);
  if (val > pow(2, NUM_BITS) - 1) {
    fprintf(stderr, "Warning, bit val is larger than attribute data type can hold\n");
  }
  att_bits temp = (att_bits)val;
  att* att = make_attributes();
  att->flag_atts = temp;
  return att;
}
