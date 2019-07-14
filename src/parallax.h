#ifndef FILE_PARALLAX_NOTSEEN
#define FILE_PARALLAX_NOTSEEN
#include "geometry.h"
#include "myList.h"
#include "graphics.h"

typedef
struct {
  //somehow, things that are in this frame
  //probably some genlist of some objects
  //maybe also a spatial hashmap. but may  be overkill
  double z_level;
} z_frame;

typedef
struct {
  //some genlist of zlevels
  gen_list* z_frames;
  //vanishing point 
  virt_pos vp;
} parallax;

//should be a geometry library thing
void virt_pos_in_new_origin(virt_pos* point, virt_pos* new, virt_pos* result);

void virt_pos_mul(virt_pos* pos, double scale, virt_pos* result);

//back to parallax
void parallax_transform_point(virt_pos* point, virt_pos* vp, double z_level , virt_pos* result);

//for drawing a polygon
void draw_parallax_polygon(camera* cam, polygon* poly, virt_pos* vp, double z_level);

//
void draw_parallax_texture(void* texture, virt_pos* vp, double z_level);


#endif
