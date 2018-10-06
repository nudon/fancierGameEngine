//so, worked on a parallax display system

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
  genlist* z_frames;
  //vanishing point 
  virt_pos vp;
} parallax;

//should be a geometry library thing
void virt_pos_in_new_origin(virt_pos* point, virt_pos* new, virt_pos* result) {
  virt_pos_sub(point, new, result);
}

void virt_pos_mul(virt_pos* pos, double scale, virt_pos* result) {
  result->x = pox->x * scale;
  result->y = pox->y * scale;
}

//back to parallax
void parallax_transform_point(virt_pos* point, virt_pos* vp, double z_level , virt_pos* result) {
  double scale = 1 / z_level;
  virt_pos loc;
  virt_pos_in_new_origin(point, vp, &loc);
  virt_pos_mul(&loc, scale, vp);
}

//for drawing a polygon
void draw_parallax_polygon(polygon* poly, virt_pos* vp, double z_level) {
  int sides = poly->sides;
  virt_pos p1, p2;
  for (int i = 0; i < sides; i++) {
    get_actual_point(poly, i, &p1);
    get_actual_point(poly, (i + 1) % sides, &p2);
    parallax_transform_point(&p1, vp, z_level, &p1);
    parallax_transform_point(&p2, vp, z_level, &p2);
    //then draw the polygon, also need a cam object here
  }
}

//
void draw_parallax_texture(void* texture, virt_pos* vp, double z_level) {
  //generall idea, have some regular sdl_render_texture call
  //take the upperleft hand point of that call, call parallax_transform_point on it
  //then, take dest sdl_rect, scale sides by 1 / z_level
  //should be it
}

