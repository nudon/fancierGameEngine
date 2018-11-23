#ifndef FILE_GRAPHICS_DEFINED
#define FILE_GRAPHICS_DEFINED
#include <SDL2/SDL_image.h>
#include "geometry.h"
#include "collider.h"

//generally don't think these should be static
//because if I want to change these while program is running, would be ?
static int SCREEN_WIDTH = 720;
static int SCREEN_HEIGHT = 480;

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

void setCam(camera* cam);

camera* getCam();

void virt_to_pixel(virt_pos* virt, pixel_pos* result);


void draw_hash_map(camera* cam, spatial_hash_map* map);

void drawWallIndication(camera* cam, SDL_Rect* rect);


void draw_line(camera* cam, pixel_pos* start, pixel_pos* end);

void draw_polygon_outline(camera* cam, polygon* poly);

void draw_bbox(camera* cam, collider* coll);

void draw_virt_pos(camera* cam, virt_pos* virt);
#endif
