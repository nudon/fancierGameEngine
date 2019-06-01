#ifndef FILE_GRAPHICS_DEFINED
#define FILE_GRAPHICS_DEFINED
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "geometry.h"
#include "collider.h"
#include "compound.h"
#include "map.h"

#define RADS_TO_DEG 360 / (2 * M_PI)

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

int getScreenWidth();
int getScreenHeight();

void setCam(camera* cam);

camera* getCam();

void virt_to_pixel(virt_pos* virt, pixel_pos* result);

void draw_events_in_map(camera* cam, map* map);
void draw_load_zones_in_map(camera* cam, map* map);
void draw_hash_map(camera* cam, spatial_hash_map* map);

void drawWallIndication(camera* cam, SDL_Rect* rect);

void draw_line(camera* cam, virt_pos* start, virt_pos* end);

void draw_polygon_outline(camera* cam, polygon* poly);

void draw_compound_outline(camera* cam, compound* comp);

void draw_compound_picture(camera* cam, compound* comp);

void draw_body_picture(camera* cam, body* body);

void draw_picture(camera* cam, picture* pic, SDL_Rect* src, SDL_Rect* dst, double rot);

void draw_bbox(camera* cam, collider* coll);

void draw_compound_bbox(camera* cam, compound* comp);

void draw_virt_pos(camera* cam, virt_pos* virt);

//texture headers

SDL_Texture* loadTexture(char* path);

SDL_Texture* loadSurfaceToTexture(SDL_Surface* loadSurf);


void drawText(TTF_Font* font, char* text, SDL_Color* textColor, SDL_Rect* dstRect);

SDL_Texture* drawTextToTexture(TTF_Font* font, char* text, SDL_Color* textColor);

SDL_Surface* createSurfaceFromDim(int w, int h);

#endif
