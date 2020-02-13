#include "game_state.h"

int quit = 0;
int mode = PLAY_MODE;

void setQuit(int new) {
  quit = new;
}

int getQuit () {
  return quit;
}

void setMode(int new) {
  mode = new;
}

int getMode() {
  return mode;
}
