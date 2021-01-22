#include "game_state.h"

static camera* gamgam;

static map* current_map = NULL;

static compound* user = NULL;

static compound* builder = NULL;

static int quit = 0;
static int mode = PLAY_MODE;

void setQuit(int new) {
  quit = new;
}

int getQuit () {
  return quit;
}

void setMode(int new) {
  mode = new;
  if (mode == PLAY_MODE) {
    if (user != NULL) {
      center_cam_on_body(get_compound_head(user));
    }
  }
  if (mode == BUILD_MODE) {
    if (builder != NULL) {
      center_cam_on_body(get_compound_head(builder));
    }
  }
}

int getMode() {
  return mode;
}

map* getMap() {
  return current_map;
}

void setMap(map* new) {
  if (current_map != new) {
    prep_for_load(new);
  }
  current_map = new;
}

void setCam(camera* cam) {
  gamgam = cam;
}

camera* getCam(){
  return gamgam;
}

compound* getUser() {
  return user;
}

void setUser(compound* new) {
  if (user != NULL && user != new) {
    fprintf(stderr, "warning, setting a new user\n");
  }
  user = new;
}

compound* getBuilder() {
  return builder;
}

void setBuilder(compound* new) {
  if (builder != NULL && builder != new) {
    fprintf(stderr, "warning, setting a new builder\n");
  }
  builder = new;
}
