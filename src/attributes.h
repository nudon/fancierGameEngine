#ifndef FILE_SEEN_ATTRIBUTES
#define FILE_SEEN_ATTRIBUTES

typedef struct att_struct att;

att* make_attributes();
void set_body_attribute(att* a);
void set_comp_attribute(att* a);

void free_attributes(att* rm);

int is_prey(att *a);
void set_prey(att *a, int val);

int is_hunter(att *a);
void set_hunter(att *a, int val);

int is_user(att *a);
void set_user(att *a, int val);

int is_travel(att *a);
void set_travel(att *a, int val);

void copy_attributes(att* src, att* dst);

char* atts_to_text(att* att);
att* text_to_atts(char* text);
    

#endif
