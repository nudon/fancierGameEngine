#include "graphics.h"

static void myDrawRect(camera* cam, int x1, int y1, int x2, int y2);
static void myDrawCirc(int x, int y, int rad);
static camera* gamgam;

static void add_surface(char* fn);
static void add_texture(char* fn);
static void init_surfaces();
static void init_textures();

SDL_Surface* get_surface_by_name(char* fn);
SDL_Texture* get_texture_by_name(char* fn);


static int SCREEN_WIDTH = 720;
static int SCREEN_HEIGHT = 480;

double x_virt_to_pixel_scale = 1.0 / 1.5;
double y_virt_to_pixel_scale = 1.0 / 2;

picture* def_pic = NULL;

static SDL_Window* gWin = NULL;

static pixel_pos* zero_pix = &((pixel_pos){.x = 0, .y = 0});

static SDL_Surface* pixel_format_holder = NULL;


static SDL_PixelFormat* get_pixel_format() {
  if (pixel_format_holder == NULL) {
    fprintf(stderr, "error, pixel format not initd\n");
    exit(3);
  }
  return pixel_format_holder->format;
}

void init_graphics() {
  SDL_Renderer* rend = NULL;
  if (init_rend(&rend) != 0) {
    fprintf(stderr, "Error, setting up SDL renderer failed, exiting\n");
    exit(1);
  }

  pixel_format_holder = createSurfaceFromDim(1,1);

  init_surfaces();
  init_textures();

  camera*  mainCam = make_camera();
  mainCam->rend = rend;
  mainCam->dest = NULL;
  setCam(mainCam);
  
  def_pic = make_picture(DEF_FN);
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
	SDL_SetRenderDrawColor(*rend, 0xff, 0xff, 0xff, SDL_ALPHA_OPAQUE);
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

picture* make_picture(char* fn) {
  picture* new = malloc(sizeof(picture));
  if (fn == NULL) {
    fn = DEF_FN;
  }
  fn = strdup(fn);
  if (get_texture_by_name(fn) == NULL) {
    add_texture(fn);
  }
  new->texture = get_texture_by_name(fn);
  new->file_name = fn;
  return new;
}

void free_picture(picture* rm) {
  SDL_DestroyTexture(rm->texture);
  free(rm->file_name);
  free(rm);
}

void set_picture_texture(picture* pic, SDL_Texture* t) {
  pic->texture = t;
  free(pic->file_name);
  pic->file_name = "MANUALLY_SET";
}

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

void draw_virt_pos(camera* cam, virt_pos* virt) {
  SDL_SetRenderDrawColor(cam->rend, 0,255,0, SDL_ALPHA_OPAQUE);
  pixel_pos pix;
  virt_to_pixel(virt, &pix);
  myDrawCirc(pix.x, pix.y, 10);
  SDL_SetRenderDrawColor(cam->rend, 0,0,255,SDL_ALPHA_OPAQUE);
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

//game constructs 

void draw_plane(plane* plane, camera* cam) {
  gen_node* curr_compound;
  compound* temp;
  SDL_SetRenderDrawColor(cam->rend,0,0,0,SDL_ALPHA_OPAQUE);
  curr_compound = get_compounds(plane)->start;
  while (curr_compound != NULL) {
    temp = (compound*)curr_compound->stored;
    draw_compound(temp, cam);
    curr_compound = curr_compound->next;
  }
  //draw_hash_map(cam, get_shm(plane)); 
}

void draw_map(camera* cam, map* map) {
  //printf("cam {.x = %i, .y = %i }\n", cam->center->x, cam->center->y);
  gen_node* curr = get_planes(map)->start;
  plane* p = NULL;
  while(curr != NULL) {
    p = (plane*)curr->stored;
    draw_plane(p, cam);
    curr = curr->next;
  }
  draw_events_in_map(cam, map);
  draw_load_zones_in_map(cam, map);
}


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
  SDL_SetRenderDrawColor(cam->rend, 0, 255, 0, SDL_ALPHA_OPAQUE);
  
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
  SDL_SetRenderDrawColor(cam->rend, 0, 0, 255, SDL_ALPHA_OPAQUE);
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

  SDL_SetRenderDrawColor(cam->rend, 255, 0, 0, SDL_ALPHA_OPAQUE);
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
      if (curr != NULL && curr->colliders_in_cell->start != NULL) {
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
  SDL_SetRenderDrawColor(cam->rend, 255, 0, 0, SDL_ALPHA_TRANSPARENT);
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

void draw_compound(compound* c, camera* cam) {
  gen_node* curr = get_bodies(c)->start;
  body* temp_body = NULL;
  polygon* temp_poly = NULL;
  collider* temp_coll = NULL;
  while(curr != NULL) {
    temp_body = (body*)curr->stored;
    temp_coll = get_collider(temp_body);
    temp_poly = get_polygon(temp_coll);

    draw_polygon_outline(cam, temp_poly);
    draw_body_picture(cam, temp_body);
    draw_events_in_list(cam, get_body_events(temp_body));
    //draw_bbox(cam, temp_coll);
    
    curr = curr->next;
  }
}

void draw_polygon_outline(camera* cam, polygon* poly) {
  int size = get_sides(poly);
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

void draw_body_picture(camera* cam, body* body) {
  collider* coll = body->coll;
  virt_pos cent = get_center(get_polygon(get_collider(body)));
  int x = x_virt_to_pixel_scale * (cent.x - get_bb_width(coll) / 2) - cam->corner.x;
  int y = y_virt_to_pixel_scale * (cent.y - get_bb_height(coll) / 2) - cam->corner.y;
  int w = x_virt_to_pixel_scale * get_bb_width(coll);
  int h = y_virt_to_pixel_scale * get_bb_height(coll);
  SDL_Rect dst = (SDL_Rect){.x = x, .y = y, .w = w, .h = h};
  SDL_RendererFlip flip = SDL_FLIP_NONE;
  vector_2 vel = *zero_vec;
  get_velocity(get_fizzle(body), &vel);
  if (vel.v1 < 0) {
    //draw picture flipped about y axis
    flip = SDL_FLIP_HORIZONTAL;
  }
  draw_picture(cam, body->pic, NULL, &dst, get_rotation(coll->shape), flip);
}

void draw_picture(camera* cam, picture* pic, SDL_Rect* src, SDL_Rect* dst, double rot, SDL_RendererFlip flip) {
  double sdl_rot = rot * RAD_2_DEG;
  if (pic == NULL) {
    pic = def_pic;
  }
  SDL_RenderCopyEx(cam->rend, pic->texture, src, dst, sdl_rot, NULL, flip);
}

void draw_bbox(camera* cam, collider* coll) {
  double orig_rot = get_rotation(coll->bbox);
  set_rotation(coll->bbox, get_rotation(coll->bbox) + get_rotation(coll->shape));
  SDL_SetRenderDrawColor(cam->rend, 255, 0, 0, SDL_ALPHA_OPAQUE);
  draw_polygon_outline(cam, coll->bbox);
  set_rotation(coll->bbox, orig_rot);
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


#define SURFACE_LIM 100
#define TEXTURE_LIM 100

static int surface_i = 0;
char* surface_name[SURFACE_LIM];
SDL_Surface* surfaces[SURFACE_LIM];

static int texture_i = 0;
char* texture_name[TEXTURE_LIM];
SDL_Texture* textures[TEXTURE_LIM];

static void add_surface(char* fn) {
  SDL_Surface* add = IMG_Load(fn);
  if (surface_i > SURFACE_LIM) {
    fprintf(stderr, "error, adding too many surfaces\n");
  }
  surface_name[surface_i] = fn;
  surfaces[surface_i] = add;
  surface_i++;
  if (get_surface_by_name(fn) == NULL) {
    fprintf(stderr, "error in adding surface %s\n", fn);
    exit(1);
  }
}

static void add_texture(char* fn) {
  SDL_Texture* add = loadTexture(fn);
  if (texture_i > TEXTURE_LIM) {
    fprintf(stderr, "error, adding too many textures\n");
  }
  texture_name[texture_i] = fn;
  textures[texture_i] = add;
  texture_i++;
  if (get_texture_by_name(fn) == NULL) {
    fprintf(stderr, "error in adding texture %s\n", fn);
    exit(1);
  }
}

static void init_surfaces() {
  surface_i = 0;
  for (int i = 0; i < SURFACE_LIM; i++) {
    surface_name[i] = NULL;
    surfaces[i] = NULL;
  }
  add_surface(SAND_FN);
}


static void init_textures() {
  texture_i = 0;
  for (int i = 0; i < TEXTURE_LIM; i++) {
    texture_name[i] = NULL;
    textures[i] = NULL;
  }
}

SDL_Surface* get_surface_by_name(char* fn) {
  for (int i = 0; i < surface_i; i++) {
    if (strcmp(fn, surface_name[i]) == 0) {
      return surfaces[i];
    }
  }
  return NULL;
}

SDL_Texture* get_texture_by_name(char* fn) {
  for (int i = 0; i < texture_i; i++) {
    if (strcmp(fn, texture_name[i]) == 0) {
      return textures[i];
    }
  }
  return NULL;
}


static int texture_number = 1;
static const int name_size = 50;

void tile_texture_for_body(body* b, char* fn, int g_w, int g_h, int t_w, int t_h) {
  SDL_Surface* tile = get_surface_by_name(fn);
  if (tile == NULL) {
    add_surface(fn);
    tile = get_surface_by_name(fn);
  }
  
  polygon* p = get_polygon(get_collider(b));
  char name[name_size];
  snprintf(name, name_size, MEDIA_FOLDER"texture_%i.png", texture_number++);
  SDL_Texture* t = generate_polygon_texture(p, g_w, g_h, tile, t_w, t_h, name);
  picture* pic = get_picture(b);
  set_picture_texture(pic, t);
  pic->file_name = strdup(name);
  
}

SDL_Texture* generate_polygon_texture(polygon* in, int g_w, int g_h, SDL_Surface* tile, int t_w, int t_h, char* surf_save_fn) {
  Uint32 interior_val = SDL_MapRGB(get_pixel_format(), 12, 34, 56);
  Uint32 exterior_val = SDL_MapRGB(get_pixel_format(), 122, 122, 20);
  Uint32 white_val = SDL_MapRGB(get_pixel_format(), 255, 255, 255);
  SDL_Surface* vignette = polygon_outline(in, interior_val, exterior_val, g_w, g_h);
  SDL_Surface* background = tile_image(vignette->w, vignette->h, tile, t_w, t_h);
  SDL_Surface* composite = createSurfaceFromDim(background->w, background->h);
  SDL_FillRect(composite, NULL, white_val);

  SDL_SetSurfaceBlendMode(vignette, SDL_BLENDMODE_NONE);
  SDL_SetSurfaceBlendMode(background, SDL_BLENDMODE_NONE);
  SDL_SetSurfaceBlendMode(composite, SDL_BLENDMODE_NONE);

  SDL_SetColorKey(vignette, SDL_TRUE, interior_val);
  SDL_BlitSurface(vignette, NULL, background, NULL);
  SDL_SetColorKey(background, SDL_TRUE, exterior_val);
  SDL_BlitSurface(background, NULL, composite, NULL);
  SDL_SetColorKey(composite, SDL_TRUE, white_val);

  
  SDL_Texture* ret = loadSurfaceToTexture(composite);
  if (surf_save_fn != NULL) {
    IMG_SavePNG(composite, surf_save_fn);
  }
  SDL_FreeSurface(vignette);
  SDL_FreeSurface(background);
  return ret;
}

SDL_Surface* tile_image(int tot_width, int tot_height, SDL_Surface* tile, int tile_width, int tile_height) {
  SDL_Surface* tot = createSurfaceFromDim(tot_width, tot_height);
  SDL_Rect rect;
  
  Uint32 white_val = SDL_MapRGB(get_pixel_format(), 255, 255, 255);
  SDL_FillRect(tot, NULL, white_val);
  
  if (tile_width <= 0) {
    tile_width = tile->w;
  }
  if (tile_height <= 0) {
    tile_height = tile->h;
  }
  for (int x_i = 0; x_i < tot_width; x_i += tile_width) {
    rect.x = x_i;
    for (int y_i = 0; y_i < tot_height; y_i += tile_height) {
      rect.w = tile_width; 
      rect.h = tile_height;
      rect.y = y_i;
      SDL_BlitScaled(tile, NULL, tot, &rect);
    }
  }
  
  return tot;
}

SDL_Surface* polygon_outline(polygon* in, Uint32 interior_val, Uint32 exterior_val, int grain_width, int grain_height) {
  polygon* p = clonePolygon(in);
  set_rotation(p,0);
  collider* c = make_collider_from_polygon(p);
  collider_ref* cr = NULL;
  spatial_hash_map* shm = NULL;
  virt_pos texture_center = *zero_pos;
  SDL_Surface* outline = NULL;
  SDL_Rect rect;
  vector* results = NULL;
  matrix_index ind;
  double xmin = 0, xmax = 0, ymin = 0, ymax = 0;
  int tot_width, tot_height, shm_cols, shm_rows;
  virt_pos p_center = get_center(p);
  extreme_projections_of_polygon(p, &p_center, x_axis, &xmin, &xmax);
  extreme_projections_of_polygon(p, &p_center, y_axis, &ymin, &ymax);
  tot_width = xmax - xmin;
  tot_height = ymax - ymin;
  
  shm_cols = (tot_width / grain_width) + 2; //add 2 for some padding on both sides
  shm_rows = (tot_height / grain_height) + 2;
  
  tot_width = shm_cols * grain_width;
  tot_height = shm_rows * grain_height;
  
  shm = create_shm(grain_width, grain_height, shm_cols, shm_rows);

  texture_center = (virt_pos){ .x = tot_width / 2, .y = tot_height / 2};
  set_center(p, &texture_center);

  cr = make_cr_from_collider(c);
  set_cr_vectors(c, cr, &(shm->cell_dim));
  entries_for_polygon(shm, p, cr->table, cr->active_cells);
  results = cr->active_cells;
  
  outline = createSurfaceFromDim(tot_width, tot_height);
  SDL_FillRect(outline, NULL, exterior_val);
  
  rect.w = grain_width;
  rect.h = grain_height;
  for (int i = 0; i < results->cur_size; i++) {
    ind = *(matrix_index*)elementAt(results, i);
    rect.x = ind.x_index * grain_width;
    rect.y = ind.y_index * grain_height;
    SDL_FillRect(outline, &rect, interior_val);
  }

  free_collider(c);
  free_shm(shm);

  return outline;
}
