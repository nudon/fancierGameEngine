#include "game_state.h"

int quit = 0;

void set_quit(int new) {
  quit = new;
}

int get_quit () {
  return quit;
}
