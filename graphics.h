#ifndef FILE_GRAPHICS_DEFINED
#define FILE_GRAPHICS_DEFINED
#include <SDL2/SDL_image.h>
#include "geometry.h"
#include "collider.h"


int getScreenWidth();
int getScreenHeight();

//renderer things
typedef
struct {
  int x;
  int y;
} pixel_pos;


typedef
struct {
  SDL_Renderer* rend;
  SDL_Rect* dest;
  pixel_pos* corner;
} camera;

extern pixel_pos* zero_pix;

void setCam(camera* cam);

camera* getCam();

void virt_to_pixel(virt_pos* virt, pixel_pos* result);


void draw_hash_map(camera* cam, spatial_hash_map* map);

void drawWallIndication(camera* cam, SDL_Rect* rect);

void draw_line(camera* cam, virt_pos* start, virt_pos* end);

void draw_polygon_outline(camera* cam, polygon* poly);

void draw_bbox(camera* cam, collider* coll);

void draw_virt_pos(camera* cam, virt_pos* virt);
#endif
