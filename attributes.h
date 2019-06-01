#ifndef FILE_SEEN_ATTRIBUTES
#define FILE_SEEN_ATTRIBUTES

typedef struct decision_att_struct decision_att;

decision_att* make_decision_att();
void free_decision_att(decision_att* rm);


int is_prey(decision_att *a);
void set_prey(decision_att *a, int val);

int is_hunter(decision_att *a);
void set_hunter(decision_att *a, int val);

int is_user(decision_att *a);
void set_user(decision_att *a, int val);

int is_travel(decision_att *a);
void set_travel(decision_att *a, int val);


char* atts_to_text(decision_att* att);
decision_att* text_to_atts(char* text);

void copy_atts(decision_att* src, decision_att* dst);
#endif
