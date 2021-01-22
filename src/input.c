#include "geometry.h"
#include "input.h"
#include "game_state.h"
#include "collider.h"
#include "guts.h"

int get_input_for_polygon(polygon* poly, vector_2* trans_disp, double* rot_disp) {
  int quit = 0;
  SDL_Event e;
  double mov_delta = 0.4;
  double jump_scale = 4.5;
  double rot_delta = 0.03;
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
	case SDLK_m:
	  setMode(BUILD_MODE);
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


/*


//trying to keep a generic polling method, which stores key states, which other, more specific functions, may interpret
enum key_state {up, down, held};
enum key_mod {none, ctrl, shift, alt};
#define KEY_LEN = 50;
key_state states[KEY_LEN];
key_mod mods[KEY_LEN];

void init_input() {
  for (int i = 0; i < KEY_LEN; i++) {
    states[i] = up;
    mods[i] = none;
  }
}

int coolder(body* b, vector_2* trans_disp, double* rot_disp) {
  //ah, well, limitations are multiple mods can't be set, and I don't think I can associate mods with keys if they are held down...
  //but, for polling, convert SDL keys to some index value, set as either keydown, keyup, or keyheld, and store associated mod state(if possible)
  //probably want a function to handle the key-index mapping
  int quit = 0;
  SDL_Event e;
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
	case SDLK_m:
	  setMode(BUILD_MODE);
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

*/
