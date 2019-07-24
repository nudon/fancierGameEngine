#include "picture.h"
#include "graphics.h"
#include <stdio.h>

picture* make_picture(char* fn) {
  picture* new = malloc(sizeof(picture));
  new->texture = loadTexture(fn);
  new->file_name = strdup(fn);
  return new;
}

void free_picture(picture* rm) {
  SDL_DestroyTexture(rm->texture);
  free(rm->file_name);
  free(rm);
}


void set_picture_texture(picture* pic, SDL_Texture* t) {
  pic->texture = t;
  free(pic->file_name);
  pic->file_name = "MANUALLY_SET";
}

