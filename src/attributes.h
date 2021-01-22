#ifndef FILE_SEEN_ATTRIBUTES
#define FILE_SEEN_ATTRIBUTES
/*
  holds basic flags for body/compound attributes
 */

typedef struct att_struct att;

//creates an uninitialized attribute
att* make_attributes();
//sets as a body attribute
void set_body_attribute(att* a);
//sets as a comp attribute
void set_comp_attribute(att* a);
//frees an attribute
void free_attributes(att* rm);

//invulneravle to damage
int is_invuln(att* a);
void set_invuln(att *a, int val);

//capable of doing damage
int is_damager(att* a);
void set_damager(att *a, int val);

//something that is hunted
int is_prey(att *a);
void set_prey(att *a, int val);

//something that hunts
int is_hunter(att *a);
void set_hunter(att *a, int val);

//controlled by the user
int is_user(att *a);
void set_user(att *a, int val);

//capable of going through maps
int is_travel(att *a);
void set_travel(att *a, int val);

//something that can be held
int is_holdable(att *a);
void set_holdable(att *a, int val);

void copy_attributes(att* src, att* dst);

//serialization helper
char* atts_to_text(att* att);
att* text_to_atts(char* text);

#endif
