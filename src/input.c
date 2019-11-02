#include "geometry.h"

#include "input.h"
#include "game_state.h"
#include "collider.h"

int get_input_for_polygon(polygon* poly, vector_2* trans_disp, double* rot_disp) {
  int quit = 0;
  SDL_Event e;
  double mov_delta = 0.4;
  double jump_scale = 4.5;
  double rot_delta = 0.03;
  //double scale_delta = 1;
  //virt_pos trans_disp = (virt_pos){.x = 0, .y = 0};
  //double rot_disp = 0;
  while (SDL_PollEvent(&e) != 0 ) {
    if (e.type == SDL_QUIT) {
      quit = 1;
    }
    else if (e.type == SDL_KEYDOWN) {
      if ((SDL_GetModState() & KMOD_CTRL)) {
	switch(e.key.keysym.sym) {
	case SDLK_UP:
	  //poly->scale += scale_delta;
	  break;
	case SDLK_DOWN:
	  //poly->scale -= scale_delta;
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
      else {
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
    }
  }
  setQuit(quit);
  return quit;
}



int get_input_for_body(body* b, vector_2* trans_disp, double* rot_disp) {
  int quit = 0;
  SDL_Event e;
  double mov_delta = 0.1;
  double rot_delta = 0.03;
  compound* comp = get_owner(b);
  int jumped = 0;
  int up = 0, down = 0, left = 0, right = 0;
  while (SDL_PollEvent(&e) != 0 ) {
    if (e.type == SDL_QUIT) {
      quit = 1;
    }
    else if (e.type == SDL_KEYDOWN) {
      if ((SDL_GetModState() & KMOD_CTRL)) {
	switch(e.key.keysym.sym) {
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
      else {
	switch(e.key.keysym.sym) {
	case SDLK_UP:
	  up = 1;
     	  break;
	case SDLK_DOWN:
	  down = 1;
	  break;
	case SDLK_RIGHT:
	  right = 1;
	  break;
	case SDLK_LEFT:
	  left = 1;
	  break;
	default:
	  break;
	}
      }
    }
  }

  const Uint8 *state = SDL_GetKeyboardState(NULL);
  if (state[SDL_SCANCODE_UP]) {
    up = 1;
  }
  if (state[SDL_SCANCODE_RIGHT]) {
    right = 1;
  }
  if (state[SDL_SCANCODE_DOWN]) {
    down = 1;
  }
  if (state[SDL_SCANCODE_LEFT]) {
    left = 1;
  }

  if (up) {
    jump_action(comp);
    jumped = 1;
  }
  if (down) {
    trans_disp->v2 += mov_delta;
  }
  if (left) {
    trans_disp->v1 -= mov_delta;    
  }
  if (right) {
    trans_disp->v1 += mov_delta;
  }
  

  if (!jumped) {
    end_jump(get_owner(b));
  }
  setQuit(quit);
  return quit;
}
