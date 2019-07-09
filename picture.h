#ifndef FILE_PICTURE_SEEN
#define FILE_PICTURE_SEEN

#include <SDL2/SDL_image.h>


typedef struct {
  SDL_Texture* texture;
  char* file_name;
} picture;

picture* make_picture(char* fn);
void free_picture(picture* rm);

#endif
