#ifndef FILE_GRAPHICS_DEFINED
#define FILE_GRAPHICS_DEFINED


/*
  Deals with drawing game objects to the screen
  
  pixel pos is something that is equivalent to a pixel on the screen, and some conversion ratios allows virt_pos to either be bigger, smaller, or the same size as pixel positions.

  cameras hold a renderer and some information which can center on a game object as it moves through the map, and determines what sub-region of the map actually is in view of the camera.

  also holds lots of special draw routines for pictures, and more invisible/hidden things like hash maps and collider bounding boxes.

  also holds some internal texture libraries
*/
typedef struct pixel_pos_struct pixel_pos;
typedef struct camera_struct camera;
typedef struct picture_struct picture;

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "geometry.h"
#include "collider.h"
#include "compound.h"
#include "map.h"
#include "plane.h"
#include "media_names.h"


struct pixel_pos_struct{
  int x;
  int y;
};

struct camera_struct {
  SDL_Renderer* rend;
  SDL_Rect* dest;
  pixel_pos corner;
  virt_pos* center;
};

struct picture_struct {
  SDL_Texture* texture;
  char* file_name;
};

extern int FPS;

//creation/free/setter for base renderable 
picture* make_picture(char* fn);
void free_picture(picture* rm);
void set_picture_texture(picture* pic, SDL_Texture* t);

//initialization and cleanup for internal globals
//init rend mostly deals with SDL setup
void init_graphics();
void quit_graphics();
int init_rend(SDL_Renderer** rend);

//creates a blank camera
camera* make_camera();
//sets camera to track center of body
void center_cam_on_body(body* body);
//sets camera to track cent
void set_camera_center(camera* cam, virt_pos* cent);
//updates the position of upper left mos corner of camera
void update_corner(camera* cam);
//returns global vals
int getScreenWidth();
int getScreenHeight();
//converts what a virtual position to a pixel position
void virt_to_pixel(virt_pos* virt, pixel_pos* result);

//basic geometry

void draw_virt_pos(camera* cam, virt_pos* virt);
void draw_line(camera* cam, virt_pos* start, virt_pos* end);

//game constructs

void draw_plane(camera* cam, plane* plane);
void draw_map(camera* cam, map* map);
void draw_events_in_map(camera* cam, map* map);
void draw_events_in_list(camera* cam, gen_list* list);
void draw_tethers_in_list(camera* cam, gen_list* list);
void draw_load_zones_in_map(camera* cam, map* map);
void draw_hash_map(camera* cam, spatial_hash_map* map);
void drawWallIndication(camera* cam, SDL_Rect* rect);
void draw_compound(camera* cam, compound* c);
void draw_polygon_outline(camera* cam, polygon* poly);
void draw_body_picture(camera* cam, body* body);
void draw_picture(camera* cam, picture* pic, SDL_Rect* src, SDL_Rect* dst, double rot, SDL_RendererFlip flip);
void draw_bbox(camera* cam, collider* coll);

// textures loading/management
SDL_Texture* loadTexture(char* path);
SDL_Texture* loadSurfaceToTexture(SDL_Surface* loadSurf);
void drawText(TTF_Font* font, char* text, SDL_Color* textColor, SDL_Rect* dstRect);
SDL_Texture* drawTextToTexture(TTF_Font* font, char* text, SDL_Color* textColor);
SDL_Surface* createSurfaceFromDim(int w, int h);

//sort of dynamically applies a texture to body
//g_* is grain dimension, determines how fine the mask is
//t_* is texture dimenisons, determines size of a single tile
//tile texture deals more with managing texture
//generate polygon textures...creates the polygon texture
void tile_texture_for_body(body* b, char* fn, int g_w, int g_h, int t_w, int t_h);
SDL_Texture* generate_polygon_texture(polygon* in, int grain_w, int grain_h, SDL_Surface* tile, int tile_width, int tile_height, char* surf_save_fn);
//creates a grid-tiling of tile
SDL_Surface* tile_image(int tot_width, int tot_height, SDL_Surface* tile, int tile_width, int tile_height);
//creates a drawing of the interior/exterior of a polygon
SDL_Surface* polygon_outline(polygon* in, Uint32 interior_val, Uint32 exterior_val, int grain_width, int grain_height);


#endif
