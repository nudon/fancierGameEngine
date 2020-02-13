#ifndef FILE_MYSHAPES_SEEN
#define FILE_MYSHAPES_SEEN

#include "compound.h"
#include "sizes.h"

body* blankBody(polygon* base);
body* makeNormalBody(int sides, double scale);
body* makeRectangleBody(int width, int height);
body* quick_nopic_block(int width, int height);
body* quick_block(int width, int height, char* fn);
body* quick_tile_block(int width, int height, char* fn); 

//functions for automatically creating/spacing multiple objects
#define HORZ_CHAIN 0
#define VERT_CHAIN 1

// duplicates a body in the specified way and returns compound, original body is not used
//also makes compound uniform, meaning bodies rotate and move in sync
compound* makeBlockChain(int pos_x, int pos_y, int width, int height, char* pic_fn, int len, int chain_type); 
compound* makeBodyChain(body* start, virt_pos* start_pos,  int len, vector_2* dir, double disp);

polygon* vision_cone(int radius, double theta_deg, int steps, double rot_off);
polygon* vision_triangle(int base, int depth, double rot_off);

//just some standard map transitions, 
polygon* make_event_poly(polygon* shape);

//blocks are just rectangles are meant to be used as floor/wall tiles
body* makeBlock (int width, int height);

#endif
