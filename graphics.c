#include "graphics.h"

static void myDrawRect(camera* cam, int x1, int y1, int x2, int y2);


static void myDrawCirc(int x, int y, int rad);

static double mSq(double b);

static void myDrawFunRect(int x1 , int y1, int x2, int y2, int layers);


static camera* gamgam;

void setCam(camera* cam) {
  gamgam = cam;
}

camera* getCam(){
  return gamgam;
}

//generally don't think these should be static
//because if I want to change these while program is running, would be ?
static int SCREEN_WIDTH = 720;
static int SCREEN_HEIGHT = 480;
//defined as the resolution in which virt_pos and pix_pos are identical
static int ORIG_SCREEN_WIDTH = 720;
static int ORIG_SCREEN_HEIGHT = 480;

//so fun thing
//if I want to support non whatever ^^^ resolution ratio is
//need a virt_to_pixel scale for both axis
double virt_to_pixel_scale = 1;

//update these on screen resolution change
double x_virt_to_pixel_scale = 1;
double y_virt_to_pixel_scale = 1;
//double x_virt_to_pixel_scale = SCREEN_WIDTH / ORIG_SCREEN_WIDTH;
//double y_virt_to_pixel_scale = SCREEN_HEIGHT / ORIG_SCREEN_HEIGHT;

int getScreenWidth() {
  return SCREEN_WIDTH;
}

int getScreenHeight(){
  return SCREEN_HEIGHT;
}



void virt_to_pixel(virt_pos* virt, pixel_pos* result) {
  result->x = virt->x * virt_to_pixel_scale;
  result->y = virt->y * virt_to_pixel_scale;
}


void draw_hash_map(camera* cam, spatial_hash_map* map) {
  //need to draw grid outlines, and draw a cross hatch for cells with things in hem
  SDL_SetRenderDrawColor(cam->rend, 255, 0, 0, 0);
  matrix_index ind;
  int rows = map->matrix_dim.height;
  int cols = map->matrix_dim.width;

  spatial_map_cell* curr;
  SDL_Rect rect;
  rect.w = map->cell_dim.width;
  rect.h = map->cell_dim.height;
  int map_height = rows * map->cell_dim.height;
  int map_width = cols * map->cell_dim.width;
  virt_pos vs, vf;
  pixel_pos ps, pf;
  //draw grid outline of cell
  //draw horizontal lines
  for (int row_i = 1; row_i  <= rows; row_i++) {
    int y = row_i * rect.h;
    vs.x = 0;
    vs.y = y;
    vf.x = map_width;
    vf.y = y;
    virt_to_pixel(&vs, &ps);
    virt_to_pixel(&vf, &pf);
    draw_line(cam, &ps, &pf);
    //SDL_RenderDrawLine( x, 0, x, map_height;
  }
  //draw vertical lines
  for (int col_i = 1; col_i <= cols; col_i++) {
    int x = col_i * rect.w;
    vs.x = x;
    vs.y = 0;
    vf.x = x;
    vf.y = map_height;

    virt_to_pixel(&vs, &ps);
    virt_to_pixel(&vf, &pf);
    draw_line(cam, &ps, &pf);
    //SDL_RenderDrawLine(0 , y, map_width, y);
  }
  //shade occupied cells
  for (int row_i = 0; row_i < rows; row_i++) {
    for (int col_i = 0; col_i < cols; col_i++) {
      //instead of loading texture, load some custom graphic or draw one
      ind.x_index = col_i;
      ind.y_index = row_i;
      curr = get_entry_in_shm(map, &ind);
      if (curr->colliders_in_cell->start != NULL) {
	//stuff in cell
	//just need a sdl rect for region to draww,
	//convert cell units from virt to pixel
	rect.x = rect.w * col_i;
	rect.y = rect.h * row_i;
	drawWallIndication(cam, &rect);
      }
    }
  }
}

void drawWallIndication(camera* cam, SDL_Rect* rect) {
  //assumes dest is in virt_pos coordinates
  SDL_SetRenderDrawColor(cam->rend, 255, 0, 0, 0);
  int xOff, yOff;
  int numLines = 3;
  virt_pos vs, vf;
  pixel_pos  ps, pf;
  
  for (int p = 0; p <= numLines; p++) {
    //also, potentiall hte dest rect is not square, so no singular offset

    xOff = rect->w * p / numLines;
    yOff = rect->h * p / numLines;

    vs.x = rect->x;
    vs.y = rect->y + rect->h - yOff;
    vf.x = rect->x + xOff;
    vf.y = rect->y + rect->h;
    virt_to_pixel(&vs, &ps);
    virt_to_pixel(&vf, &pf);
    draw_line(cam, &ps, &pf);

    //there were two loops in the orig draw indicator for ???
    vs.x = rect->x + xOff;
    vs.y = rect->y;
    vf.x = rect->x + rect->w;
    vf.y = rect->y + rect->h - yOff;
    virt_to_pixel(&vs, &ps);
    virt_to_pixel(&vf, &pf);
    draw_line(cam, &ps, &pf);

  }
  //for (int p = 0; p <= numLines; p++) {
    //pOff = TILED * p / numLines;
    
        //pOff = TILED * p / numLines;
    //c1 (x, y + h - poff
    //c2,(x + poff, yq
    //SDL_RenderDrawLine(rend, dest->x, dest->y + dest->h - pOff,
    //dest->x + pOff, dest->y + dest->h);
    //SDL_RenderDrawLine(rend, dest->x + pOff, dest->y,
    //dest->x + dest->w, dest->y + dest->h - pOff);
  //}
}

void draw_virt_pos(camera* cam, virt_pos* virt) {
  SDL_SetRenderDrawColor(cam->rend, 0,255,0, 0);
  pixel_pos pix;
  virt_to_pixel(virt, &pix);
  myDrawCirc(pix.x, pix.y, 10);
  SDL_SetRenderDrawColor(cam->rend, 0,0,255,0);
  myDrawCirc(pix.x, pix.y, 9);
}

void draw_line(camera* cam, pixel_pos* start, pixel_pos* end) {
  pixel_pos o = *(cam->corner);
  SDL_RenderDrawLine(cam->rend,
		     start->x - o.x, start->y - o.y,
		     end->x - o.x, end->y - o.y);
}

void draw_polygon_outline(camera* cam, polygon* poly) {
  //grab polygon points, rotate, offset by origin
  int size = poly->sides;
  virt_pos points[size];
  for (int i = 0; i < size; i++) {
    get_actual_point(poly, i, &(points[i]));
  }
  //convert to pixel_pos
  //then call some more camera draw primitives which works out where or if to draw object
  pixel_pos ts, tf;
  for (int i = 0; i < size; i++) {
    virt_to_pixel(&(points[i]), &ts);
    virt_to_pixel(&(points[(i + 1) % size]), &tf);
    draw_line(cam, &ts, &tf);
  }
}

void draw_bbox(camera* cam, collider* coll) {
  double orig_rot = coll->bbox->rotation;
  coll->bbox->rotation += coll->shape->rotation;
  coll->bbox->center = coll->shape->center;
  SDL_SetRenderDrawColor(cam->rend, 255, 0, 0, 0);
  draw_polygon_outline(cam, coll->bbox);
  coll->bbox->rotation = orig_rot;
}


// //
//stuff from m02


void myDrawRect(camera* cam, int x1, int y1, int x2, int y2) {
  SDL_Rect rect;
  rect.x = x1;
  rect.y = y1;
  rect.w = x2 - x1;
  rect.h = y2 - y1;
  SDL_RenderFillRect(getCam()->rend, &rect);
}


void myDrawCirc(int x, int y, int rad) {
  double theta = 0;
  double halfACircle = M_PI;
  int quality = 100;
  double dTheta = M_PI / quality;
  int c1x, c2x, c1y, c2y;
  while(theta < halfACircle) {
    c1x = x + rad * cos(theta);
    c1y = y + rad * sin(theta);
    c2x = x;
    c2y = y - rad * sin(theta);
    myDrawRect(getCam(),c1x, c1y, c2x, c2y);
    theta += dTheta;
  }

}

double mSq(double b) {
  return b*b;
}

void myDrawFunRect(int x1 , int y1, int x2, int y2, int layers) {
  if (layers > 0) {
    SDL_Color old;
    camera* cam = getCam();
    SDL_Renderer* rend = cam->rend;
    if (SDL_GetRenderDrawColor(rend, &old.r, &old.g, &old.b, &old.a) == 0) {
      int r;
      int g;
      int b;
      int scrmble = layers;
      int dx, dy;
      dx = ((x2 - x1) / 2) / layers;
      dy = ((y2 - y1) / 2) / layers;
      for (int i = 0; i <  layers; i++) {
	r = 255 * mSq(sin(i * M_PI/scrmble));
	r = r << (sizeof(int) / 2);
	g = 255 * mSq(cos(i * M_PI/scrmble));
	g = g << (sizeof(int) / 2);
	b = 255 * mSq(tan(i * M_PI/scrmble));
	b = b << (sizeof(int) / 2);
	SDL_SetRenderDrawColor( rend, r, g, b, 0xFF );
	myDrawRect(cam, x1 + dx * i, y1 + dy * i, x2 - dx * i, y2 - dy * i);
      }
      SDL_SetRenderDrawColor(rend, old.r, old.g, old.b, old.a);
    }
    else {
      fprintf(stderr, "Unable to backup previous drawRender settings: %s  \n", SDL_GetError());
    }
  }
  else {
    fprintf(stderr, "FunRect passed negative or zero layers \n");
  }
}
