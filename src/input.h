#ifndef FILE_INPUT_NOTSEEN
#define FILE_INPUT_NOTSEEN

#include <SDL2/SDL_image.h>
#include "body.h"
int get_input_for_polygon(polygon* poly, vector_2* trans_disp, double* rot_disp);
int get_input_for_body(body* b, vector_2* trans_disp, double* rot_disp);

#endif
