#ifndef FILE_SEEN_FLAGS
#define FILE_SEEN_FLAGS
/*
  holds basic flags for body/compound attributes
 */

typedef struct flags_struct flags;

//creates an uninitialized attribute
flags* make_flags();

void flags_set_type_body(flags* a);

void flags_set_type_comp(flags* a);

void flags_set_type_draw(flags* a);

void free_flags(flags* rm);

//invulneravle to damage
int is_invuln(flags* a);
void set_invuln(flags *a, int val);

//capable of doing damage
int is_damager(flags* a);
void set_damager(flags *a, int val);

//something that is hunted
int is_prey(flags *a);
void set_prey(flags *a, int val);

//something that hunts
int is_hunter(flags *a);
void set_hunter(flags *a, int val);

//controlled by the user
int is_user(flags *a);
void set_user(flags *a, int val);

//capable of going through maps
int is_travel(flags *a);
void set_travel(flags *a, int val);

//something that can be held
int is_holdable(flags *a);
void set_holdable(flags *a, int val);

int is_draw_outline(flags* a);
void set_draw_outline(flags* a, int val);

int is_draw_picture(flags* a);
void set_draw_picture(flags* a, int val);

int is_draw_events(flags* a);
void set_draw_events(flags* a, int val);

int is_draw_bbox(flags* a);
void set_draw_bbox(flags* a, int val);


void copy_flags(flags* src, flags* dst);

//serialization helper
char* flags_to_text(flags* f);
flags* text_to_flags(char* text);

#endif
