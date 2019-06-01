#ifndef FILE_PICTURE_SEEN
#define FILE_PICTURE_SEEN

#include <SDL2/SDL_image.h>

//just a basic thing holding a texture for now
//in future, can have a static dictionary of textures so multiple objects using 1 texture
//all point to the same SDL texture
//can also work in sprite/animation states by defining a taking rendering sub-regions of the textures;


typedef struct {
  SDL_Texture* texture;
  char* file_name;
} picture;

picture* make_picture(char* fn);
void free_picture(picture* rm);

#endif
