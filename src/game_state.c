#include "game_state.h"

int quit = 0;

void setQuit(int new) {
  quit = new;
}

int getQuit () {
  return quit;
}
