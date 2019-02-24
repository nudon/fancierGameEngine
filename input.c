#include "geometry.h"

#include "input.h"
#include "game_state.h"
int get_input_for_polygon(polygon* poly, vector_2* trans_disp, double* rot_disp) {
  int quit = 0;
  SDL_Event e;
  double mov_delta = .4;
  double rot_delta = .3;
  double scale_delta = 1;
  //virt_pos trans_disp = (virt_pos){.x = 0, .y = 0};
  //double rot_disp = 0;
  while (SDL_PollEvent(&e) != 0 ) {
    if (e.type == SDL_QUIT) {
      quit = 1;
    }
    else if (e.type == SDL_KEYDOWN) {
      if (!(SDL_GetModState() & KMOD_CTRL)) {
	switch(e.key.keysym.sym) {
	case SDLK_UP:
	  trans_disp->v2 -= mov_delta;
	  break;
	case SDLK_DOWN:
	  trans_disp->v2 += mov_delta;
	  break;
	case SDLK_RIGHT:
	  trans_disp->v1 += mov_delta;
	  break;
	case SDLK_LEFT:
	  trans_disp->v1 -= mov_delta;
	  break;
	default:

	  break;
	    
	}
      }
      else {
	switch(e.key.keysym.sym) {
	  //scaling is bad thing to do with the shm being a thing
	  //will probably crash when increasing size too much and generating entries for things
	case SDLK_UP:
	  poly->scale += scale_delta;
	  break;
	case SDLK_DOWN:
	  poly->scale -= scale_delta;
	  break;
	case SDLK_RIGHT:
	  *rot_disp += rot_delta;
	  break;

	case SDLK_LEFT:
	  *rot_disp -= rot_delta;
	  break;
	default:

	  break;
	}
      }
    }
  }
  //printf("Input: trans is (%d, %d) and rot is %f\n", trans_disp->x, trans_disp->y, *rot_disp);
  set_quit(quit);
  return quit;
}
