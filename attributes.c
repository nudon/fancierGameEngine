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

typedef uint8_t att_bits;
#define NUM_BITS 8
struct decision_att_struct {
  att_bits flag_atts;
};



decision_att* make_decision_att() {
  decision_att* new = calloc(1,sizeof(decision_att));
  return new;
}

void free_decision_att(decision_att* rm) {
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

int is_prey(decision_att *a) {
  return test_bit(a->flag_atts, PREY_BIT);
}
void set_prey(decision_att *a, int val) {
  set_bit(&(a->flag_atts), PREY_BIT, val);
}

int is_hunter(decision_att *a) {
  return test_bit(a->flag_atts, HUNTER_BIT);
}
void set_hunter(decision_att *a, int val) {
  set_bit(&(a->flag_atts), HUNTER_BIT, val);
}

int is_user(decision_att *a) {
  return test_bit(a->flag_atts, USER_BIT);
}
void set_user(decision_att *a, int val) {
  set_bit(&(a->flag_atts), USER_BIT, val);
}

int is_travel(decision_att *a) {
  return test_bit(a->flag_atts, TRAVEL_BIT);
}
void set_travel(decision_att *a, int val) {
  set_bit(&(a->flag_atts), TRAVEL_BIT, val);
}


char* atts_to_text(decision_att* att) {
  char* text = malloc(sizeof(char*) * (NUM_BITS + 1)) ;
  snprintf(text, NUM_BITS, "%d", att->flag_atts);
  return text;
}

decision_att* text_to_atts(char* text) {
  long val = atoi(text);
  if (val > pow(2, NUM_BITS)) {
    fprintf(stderr, "Warning, bit val is larger than attribute data type can hold\n");
  }
  att_bits temp = (att_bits)val;
  decision_att* att = make_decision_att();
  att->flag_atts = temp;
  return att;
}
    
void copy_atts(decision_att* src, decision_att* dst) {
  dst->flag_atts = src->flag_atts;
}
