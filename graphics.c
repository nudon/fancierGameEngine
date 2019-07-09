#include "graphics.h"



static void myDrawRect(camera* cam, int x1, int y1, int x2, int y2);
static void myDrawCirc(int x, int y, int rad);
static camera* gamgam;

static int SCREEN_WIDTH = 720;
static int SCREEN_HEIGHT = 480;

picture* def_pic = NULL;

static SDL_Window* gWin = NULL;

static pixel_pos* zero_pix = &((pixel_pos){.x = 0, .y = 0});

void init_graphics() {
  SDL_Renderer* rend = NULL;
  if (init_rend(&rend) != 0) {
    fprintf(stderr, "Error, setting up SDL renderer failed, exiting\n");
    exit(1);
  }

  camera*  mainCam = make_camera();
  mainCam->rend = rend;
  mainCam->dest = NULL;
  setCam(mainCam);
  
  def_pic = make_picture("./media/rad.png");
  if (def_pic->texture == NULL) {
    def_pic = make_picture("./media/def.png");
      if (def_pic->texture == NULL) {
	fprintf(stderr, "missing both of the default textures, exiting\n");
	fprintf(stderr, "place a random picture in media dir and call it \"def.png\" to fix\n");
	exit(3);
      }
  }
  //also probably put texture loading in here
}

void quit_graphics() {
  SDL_DestroyWindow(gWin);
  gWin = NULL;
  //also free any textures that got loaded
  
}

int init_rend(SDL_Renderer** rend) {
  int fail = 0;
  if(SDL_Init(SDL_INIT_VIDEO) >= 0) {
    gWin = SDL_CreateWindow("SDL, Now with renderererers",
			    SDL_WINDOWPOS_UNDEFINED,
			    SDL_WINDOWPOS_UNDEFINED,
			    getScreenWidth(),
			    getScreenHeight(),
			    SDL_WINDOW_SHOWN);
    if (gWin != NULL) {
      *rend = SDL_CreateRenderer(gWin, -1, SDL_RENDERER_ACCELERATED);
      if (rend != NULL) {
	SDL_SetRenderDrawColor(*rend, 0xff, 0xff, 0xff, 0xff);
	int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
	if ((IMG_Init(imgFlags) & imgFlags)) {
	  if (TTF_Init() != -1) {
	    //initialize my libraries
	    init_poltergeists();
	    init_events();
	    init_map_load();
	  }
	  else {
	    fail = 5;
	    fprintf(stderr, "Error in loading TTF libraries: %s\n",
		    TTF_GetError());
	  }
	}
	else {
	  fail = 4;
	  fprintf(stderr, "Error in loading img loading librarys: %s \n",
		  IMG_GetError());
	}
      }
      else {
	fail = 3;
	fprintf(stderr, "Error in creating renderer for gWin: %s \n",
		SDL_GetError());
      }      
    }
    else {
      fail = 2;
      fprintf(stderr, "Window Could not be created: %s\n",
	      SDL_GetError());
    }
  }
  else {
    fprintf(stderr, "Error in init video %s\n",
	    SDL_GetError() );
    fail =1;
  }
  return fail;
}

double x_virt_to_pixel_scale = 1.02;
double y_virt_to_pixel_scale = 1.07;
//double x_virt_to_pixel_scale = SCREEN_WIDTH / ORIG_SCREEN_WIDTH;
//double y_virt_to_pixel_scale = SCREEN_HEIGHT / ORIG_SCREEN_HEIGHT;

camera* make_camera() {
  camera* cam = malloc(sizeof(camera));
  cam->rend = NULL;
  cam->dest = NULL;
  cam->center = NULL;
  cam->corner = *zero_pix;
  return cam;
}

void setCam(camera* cam) {
  gamgam = cam;
}

camera* getCam(){
  return gamgam;
}

void set_camera_center(camera* cam, virt_pos* cent) {
  cam->center = cent;
}

void update_corner(camera* cam) {
  pixel_pos temp = *zero_pix;
  if (cam->center != NULL) {
    virt_to_pixel(cam->center, &temp);
    temp.x -= getScreenWidth() / 2;
    temp.y -= getScreenHeight() / 2;
  }
  cam->corner = temp;
}


int getScreenWidth() {
  return SCREEN_WIDTH;
}

int getScreenHeight(){
  return SCREEN_HEIGHT;
}



void virt_to_pixel(virt_pos* virt, pixel_pos* result) {
  result->x = virt->x * x_virt_to_pixel_scale;
  result->y = virt->y * y_virt_to_pixel_scale;
}

//basic geometry

void draw_events_in_map(camera* cam, map* map) {
  plane* aPlane = NULL;
  gen_node* plane_node = get_planes(map)->start;
  while (plane_node != NULL) {
    aPlane = (plane*)plane_node->stored;
    draw_events_in_list(cam, get_events(aPlane));
    plane_node = plane_node->next;
  }
}

void draw_events_in_list(camera* cam, gen_list* list) {
  SDL_SetRenderDrawColor(cam->rend, 0, 255, 0, 0);
  
  event* e = NULL;
  polygon* p = NULL;
  gen_node* event_node = NULL;
  
  event_node = list->start;  
  while(event_node != NULL) {
    e = (event*)event_node->stored;
    p = get_polygon(get_event_collider(e));
    draw_polygon_outline(cam, p);
    event_node = event_node->next;
  }
  
}


void draw_load_zones_in_map(camera* cam, map* map) {
  SDL_SetRenderDrawColor(cam->rend, 0, 0, 255, 0);
  plane* aPlane = NULL;
  load_zone* lz = NULL;
  polygon* p = NULL;
  gen_node* lz_node = NULL;
  gen_node* plane_node = get_planes(map)->start;
  while (plane_node != NULL) {
    aPlane = (plane*)plane_node->stored;
    lz_node = get_load_zones(aPlane)->start;
    
    while(lz_node != NULL) {
      lz = (load_zone*)lz_node->stored;
      p = get_polygon(get_event_collider(get_lz_event(lz)));
      draw_polygon_outline(cam, p);
      lz_node = lz_node->next;
    }
    plane_node = plane_node->next;
  }
}

void draw_hash_map(camera* cam, spatial_hash_map* map) {

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
  //draw grid outline of cell
  //draw horizontal lines
  for (int row_i = 1; row_i  <= rows; row_i++) {
    int y = row_i * rect.h;
    vs.x = 0;
    vs.y = y;
    vf.x = map_width;
    vf.y = y;
    draw_line(cam, &vs, &vf);
    //SDL_RenderDrawLine( x, 0, x, map_height;
  }
  //draw vertical lines
  for (int col_i = 1; col_i <= cols; col_i++) {
    int x = col_i * rect.w;
    vs.x = x;
    vs.y = 0;
    vf.x = x;
    vf.y = map_height;
    draw_line(cam, &vs, &vf);
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
  for (int p = 0; p <= numLines; p++) {
    xOff = rect->w * p / numLines;
    yOff = rect->h * p / numLines;

    vs.x = rect->x;
    vs.y = rect->y + rect->h - yOff;
    vf.x = rect->x + xOff;
    vf.y = rect->y + rect->h;
    draw_line(cam, &vs, &vf);

    vs.x = rect->x + xOff;
    vs.y = rect->y;
    vf.x = rect->x + rect->w;
    vf.y = rect->y + rect->h - yOff;
    draw_line(cam, &vs, &vf);

  }
}


void draw_virt_pos(camera* cam, virt_pos* virt) {
  SDL_SetRenderDrawColor(cam->rend, 0,255,0, 0);
  pixel_pos pix;
  virt_to_pixel(virt, &pix);
  myDrawCirc(pix.x, pix.y, 10);
  SDL_SetRenderDrawColor(cam->rend, 0,0,255,0);
  myDrawCirc(pix.x, pix.y, 9);
}

void draw_line(camera* cam, virt_pos* start, virt_pos* end) {
  pixel_pos o = cam->corner;
  pixel_pos t1 = *zero_pix, t2 = *zero_pix;
  virt_to_pixel(start, &t1);
  virt_to_pixel(end, &t2);
  SDL_RenderDrawLine(cam->rend,
		     t1.x - o.x, t1.y - o.y,
		     t2.x - o.x, t2.y - o.y);
}

void draw_polygon_outline(camera* cam, polygon* poly) {
  int size = poly->sides;
  virt_pos points[size];
  for (int i = 0; i < size; i++) {
    get_actual_point(poly, i, &(points[i]));
  }
  //convert to pixel_pos
  //then call some more camera draw primitives which works out where or if to draw object
  virt_pos p1 = *zero_pos, p2 = *zero_pos;
  for (int i = 0; i < size; i++) {
    p1 = points[i];
    p2 = points[(i + 1) % size];
    draw_line(cam, &p1, &p2);
  }
}

void draw_compound_outline(camera* cam, compound* comp){
  gen_node* curr = get_bodies(comp)->start;
  body* temp;
  while (curr != NULL) {
    temp = (body*)curr->stored;
    draw_polygon_outline(cam, temp->coll->shape);
    curr = curr->next;
  }
}

void draw_compound_picture(camera* cam, compound* comp) {
  gen_node* curr = get_bodies(comp)->start;
  body* temp;
  while (curr != NULL) {
    temp = (body*)curr->stored;
    draw_body_picture(cam, temp);
    curr = curr->next;
  }  

}

void draw_body_picture(camera* cam, body* body) {
  collider* coll = body->coll;
  virt_pos cent = *getCenter(body);
  int x = x_virt_to_pixel_scale * (cent.x - get_bb_width(coll) / 2) - cam->corner.x;
  int y = y_virt_to_pixel_scale * (cent.y - get_bb_height(coll) / 2) - cam->corner.y;
  int w = x_virt_to_pixel_scale * get_bb_width(coll);
  int h = y_virt_to_pixel_scale * get_bb_height(coll);
  SDL_Rect dst = (SDL_Rect){.x = x, .y = y, .w = w, .h = h};
  draw_picture(cam, body->pic, NULL, &dst, get_rotation(coll->shape));
}

void draw_picture(camera* cam, picture* pic, SDL_Rect* src, SDL_Rect* dst, double rot) {
  //polygon rotation is in radians and + is counterclockwise
  //SDL expects degress and + is clockwise
  double sdl_rot = rot * RADS_TO_DEG * 1;
  if (pic == NULL) {
    pic = def_pic;
  }
  SDL_RenderCopyEx(cam->rend, pic->texture, src, dst, sdl_rot, NULL, SDL_FLIP_NONE);
}

void draw_bbox(camera* cam, collider* coll) {
  double orig_rot = get_rotation(coll->bbox);
  set_rotation(coll->bbox, get_rotation(coll->bbox) + get_rotation(coll->shape));
  SDL_SetRenderDrawColor(cam->rend, 255, 0, 0, 0);
  draw_polygon_outline(cam, coll->bbox);
  set_rotation(coll->bbox, orig_rot);
  //coll->bbox->rotation = orig_rot;
}

void draw_compound_bbox(camera* cam, compound* comp) {
  gen_node* curr = get_bodies(comp)->start;
  body* temp;
  while (curr != NULL) {
    temp = (body*)curr->stored;
    draw_bbox(cam, temp->coll);
    curr = curr->next;
  }
}

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

// textures


SDL_Texture* loadTexture(char* path) {
  SDL_Texture* newText = IMG_LoadTexture(getCam()->rend, path);
  if (newText == NULL) {
    fprintf(stderr, "Error in converting surface to Texture: %s \n",
	    SDL_GetError());
  }
  return newText;
}

SDL_Texture* loadSurfaceToTexture(SDL_Surface* loadSurf) {
  SDL_Texture* newText;
  newText = SDL_CreateTextureFromSurface(getCam()->rend, loadSurf);
  if (newText == NULL) {
    fprintf(stderr, "Error in converting surface to Texture: %s \n",
	    SDL_GetError());
    newText = NULL;
  }
  return newText;
}


void drawText(TTF_Font* font, char* text, SDL_Color* textColor, SDL_Rect* dstRect) {
  SDL_Texture* texture = drawTextToTexture(font, text, textColor);
  SDL_RenderCopy(getCam()->rend, texture, NULL, dstRect);
  SDL_DestroyTexture(texture);
}

SDL_Texture* drawTextToTexture(TTF_Font* font, char* text, SDL_Color* textColor) {
  SDL_Surface* textSurf;
  SDL_Texture* textText;
  textSurf = TTF_RenderText_Solid(font, text, *textColor);
  textText = loadSurfaceToTexture(textSurf);
  SDL_FreeSurface(textSurf);
  return textText;
}

SDL_Surface* createSurfaceFromDim(int w, int h) {
  //I stole this from https://wiki.libsdl.org/SDL_CreateRGBSurface
  SDL_Surface* surface;
  Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  rmask = 0xff000000;
  gmask = 0x00ff0000;
  bmask = 0x0000ff00;
  amask = 0x000000ff;
#else
  rmask = 0x000000ff;
  gmask = 0x0000ff00;
  bmask = 0x00ff0000;
  amask = 0xff000000;
#endif
  surface = SDL_CreateRGBSurface(0, w, h,32,rmask, gmask,  bmask, amask);
  //my crimes end here.
  return surface;
}
