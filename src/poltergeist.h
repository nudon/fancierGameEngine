#ifndef FILE_POLTERGEIST_SEEN
#define FILE_POLTERGEIST_SEEN

typedef struct poltergeist_struct poltergeist;

#include "collider.h"
#include "body.h"

//so, library is responsible for moving things


void init_poltergeists();
char* get_polt_name(poltergeist* p);
void set_polt_by_name(poltergeist* p, char* polt);

poltergeist* make_poltergeist();


void give_standard_poltergeist(poltergeist* polt);
void give_user_poltergeist(poltergeist* polt);
void give_builder_poltergeist(poltergeist* polt);

void apply_poltergeist(poltergeist* polt, struct body_struct* body, vector_2* t_disp, double* r_disp);

void no_poltergeist(struct body_struct* body, vector_2* t_disp, double* r_disp);
void user_poltergeist(struct body_struct* body, vector_2* t_disp, double* r_disp);
void builder_poltergeist(body* builder, vector_2* t_disp, double* r_disp);
void standard_poltergeist(struct body_struct* body, vector_2* t_disp, double* r_disp);
void look_poltergeist(body* body, vector_2* t_disp, double* r_disp);
void holder_poltergeist(body* b, vector_2* t_disp, double* r_disp);
void basic_brain(body* b, vector_2* t_disp, double* r_disp);

void reorient(body* b, vector_2* vec, vector_2* t_disp, double* r);
void translate(body* b, vector_2* vec, vector_2* t_disp, double* r);
#endif
