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
  double mov_delta = 0.07;
  double rot_delta = 0.03;
  compound* comp = get_owner(b);
  int jumped = 0;
  int up = 0, down = 0, left = 0, right = 0, pickup = 0, throw = 0;
  vector_2 dir = *zero_vec;
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
	case SDLK_z:
	  pickup = 1;
	  break;
	case SDLK_x:
	  throw = 1;
	  break;
	case SDLK_SPACE:
	  cut_compound(get_owner(b));
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
  if (state[SDL_SCANCODE_Z]) {
    pickup = 1;
  }
  if (state[SDL_SCANCODE_X]) {
    throw = 1;
  }

  
  if (up) {
    jump_action(comp);
    jumped = 1;
    dir.v2 -= 1;
  }
  if (down) {
    trans_disp->v2 += mov_delta;
    dir.v2 += 1;
  }
  if (left) {
    trans_disp->v1 -= mov_delta;
    dir.v1 -= 1;
  }
  if (right) {
    trans_disp->v1 += mov_delta;
    dir.v1 += 1;
  }
  if (pickup) {
    pickup_action(comp);
  }
  else if (throw) {
    throw_action(comp);
  }
  

  if (!jumped) {
    end_jump(comp);
  }
  add_to_smarts(get_compound_smarts(comp), SM_MOVE, &dir);
  setQuit(quit);
  return quit;
}
