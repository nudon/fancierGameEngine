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
  pixel_pos corner;
  virt_pos* center;
} camera;


void init_graphics();
void quit_graphics();
int init_rend(SDL_Renderer** rend);

camera* make_camera();
void setCam(camera* cam);
camera* getCam();
void set_camera_center(camera* cam, virt_pos* cent);
void update_corner(camera* cam);
int getScreenWidth();
int getScreenHeight();
void virt_to_pixel(virt_pos* virt, pixel_pos* result);

//basic geometry

void draw_virt_pos(camera* cam, virt_pos* virt);
void draw_line(camera* cam, virt_pos* start, virt_pos* end);

//game constructs

void draw_plane(plane* plane, camera* cam);
void draw_map(camera* cam, map* map);
void draw_events_in_map(camera* cam, map* map);
void draw_events_in_list(camera* cam, gen_list* list);
void draw_load_zones_in_map(camera* cam, map* map);
void draw_hash_map(camera* cam, spatial_hash_map* map);
void drawWallIndication(camera* cam, SDL_Rect* rect);
void draw_compound(compound* c, camera* cam);
void draw_polygon_outline(camera* cam, polygon* poly);
void draw_body_picture(camera* cam, body* body);
void draw_picture(camera* cam, picture* pic, SDL_Rect* src, SDL_Rect* dst, double rot);
void draw_bbox(camera* cam, collider* coll);

// textures
SDL_Texture* loadTexture(char* path);
SDL_Texture* loadSurfaceToTexture(SDL_Surface* loadSurf);
void drawText(TTF_Font* font, char* text, SDL_Color* textColor, SDL_Rect* dstRect);
SDL_Texture* drawTextToTexture(TTF_Font* font, char* text, SDL_Color* textColor);
SDL_Surface* createSurfaceFromDim(int w, int h);


SDL_Texture* outline(polygon* p);

#endif