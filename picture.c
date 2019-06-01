#include "picture.h"
#include "graphics.h"
//just a basic thing holding a texture for now
//in future, can have a static dictionary of textures so multiple objects using 1 texture
//all point to the same SDL texture
//can also work in sprite/animation states by defining a taking rendering sub-regions of the textures;

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

