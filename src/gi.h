#ifndef FILE_GI_SEEN
#define FILE_GI_SEEN

typedef struct gi_struct gi;


#include "events.h"
#include "body.h"
#include "collider.h"
#include "attributes.h"


event* make_basic_vision_event(body* b);

polygon* vision_triangle(int base, int depth, double rot_off);

void basic_decide_event(struct event_struct* e, body* b2);
void basic_decide(body* self, body* triger);


gi* create_gi();
void free_gi(gi* g);

void add_to_dir(gi* g, vector_2* dir);
void calc_new_dir(gi* g);

vector_2 get_curr_dir(gi* g);
void set_curr_dir(gi* g, vector_2* vec);
vector_2 get_new_dir(gi* g);
void set_new_dir(gi* g, vector_2* vec);
decision_att* get_gi_attributes(gi* g);
void set_gi_attributes(gi* g, decision_att* atts);
double get_exp_decay_alpha(gi* g);
void set_exp_decay_alpha(gi* g, double val);


#endif
