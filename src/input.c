#include "geometry.h"

#include "input.h"
#include "game_state.h"
#include "collider.h"

int get_input_for_polygon(polygon* poly, vector_2* trans_disp, double* rot_disp) {
  int quit = 0;
  SDL_Event e;
  double mov_delta = 100;
  double jump_scale = 4;
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
	  trans_disp->v2 -= mov_delta * jump_scale;
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
  setQuit(quit);
  return quit;
}
