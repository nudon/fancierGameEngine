#ifndef FILE_COLLIDER_SEEN
#define FILE_COLLIDER_SEEN

typedef struct spatial_hash_map_struct spatial_hash_map;
typedef struct collider_struct collider;
typedef struct collider_ref_struct collider_ref;

#include "myList.h"
#include "myVector.h"
#include "hash_table.h"
#include "myMatrix.h"
#include "geometry.h"
#include "physics.h"
#include "compound.h"

/*
dep: std libs, list/matrix libs, and graphics. 

internals:
status constants used to keep track of an object status while collisions are being processed

//proccesses matrix indexes (passed as void* because ?)
int matrix_index_compare(void* a, void* b);
int matrix_index_hash(void* i);



*/


typedef
struct {
  int width;
  int height;
} box;

typedef
struct {
  int x_index;
  int y_index;
} matrix_index;


struct collider_struct{
  box bb_dim;
  polygon* shape;
  polygon* bbox;
  struct body_struct* body;
  collider_ref* collider_node;
};

//holds preallocated things which are used by spatial hash map
struct collider_ref_struct{
  collider* collider;

  int max_ref_amount;
  //vectors which hold pre-allocated matrix indexes for collider
  vector* active_cells;
  vector* old_cells;
  //hash table to be used for making a set of matrix indexes
  hash_table* table;
  //gen nodes to put into various shm cells, points to containing collider list node
  //contains max_ref_amount gen_nodes
  gen_node** active_cell_nodes;
  //node to be used in collision resolution, points to collider
  gen_node* cr_node;
  int status;
};


typedef struct {
  gen_list * colliders_in_cell;
} spatial_map_cell;


struct spatial_hash_map_struct{
  box cell_dim;
  box matrix_dim;
  gen_matrix* hash_map;
};

//getters
polygon* get_polygon(collider* coll);
collider_ref* get_collider_ref(collider* coll);

//updates a colliders position and rotation, clears old shm entries and adds new ones. 
int update(spatial_hash_map* map, collider* coll, virt_pos* displace, double rot);
//swaps the active-cells with old cells, used after update removes active and adds old
void update_refs(collider* coll);

//inserts collider into map, creates collider_ref, calcs and add's to shm entries
void insert_collider_in_shm(spatial_hash_map* map, collider* collider);
//adds colliders within compount into map
void insert_compound_in_shm(spatial_hash_map* map, struct compound_struct* comp);
//removes colliders from compound from map, cleans entries within map, frees collider ref
void remove_compound_from_shm(spatial_hash_map* map, compound* comp);
//creates internal structures to hold shm cells for the compound
void set_cr_vectors(collider* coll, collider_ref* ref, box* cell_dim);
//calculates the max amount of cells a bounding box can take up
int calc_max_cell_span(double boxW, double boxH, double cellW, double cellH);
//calculates and stores bounding-box information for polygon
void find_bb_for_polygon(polygon* poly, polygon* result);
//calculates the width/height of a polygon, 
void fill_bb_dim(polygon* bbox, box* bb_dim);
//getters
int get_bb_width (collider* coll);
int get_bb_height (collider* coll);

//assuming the outline of a shape has been traced, and ind points within, grabs the cells that are within the shape.
void recursive_fill( hash_table* table, matrix_index ind ,vector* result);
//calculates shm entries for polygon
void entries_for_polygon(spatial_hash_map * map, polygon* p, hash_table* table, vector* result);
//calls entries_for_polygon with colliders bounding box, with some modifications to align rotation of shapes.
void entries_for_collider(spatial_hash_map * map, collider* collider, vector* result);

//working with matrix indexes
int matrix_index_difference(matrix_index* m, matrix_index* b);
int adjacent_indexes(matrix_index* m, matrix_index* b);

//removes collider from each shm-cell within entries
void remove_collider_from_shm_entries(spatial_hash_map* map, collider_ref* node, vector* entries_to_clear);
//adds collider to each shm-cell within entries
void add_collider_to_shm_entries(spatial_hash_map* map, collider_ref* node, vector* entries_to_add);

//stores the unique collider_refs found within entries into the result
int store_unique_colliders_in_list(spatial_hash_map* map, vector* entries, gen_list* result);

//given a list of collider*, reset status of collider_ref and empties list
void clean_collider_list(gen_list* list);

//creates a collider_ref based on a collider
collider_ref* make_cr_from_collider(collider* coll);

//returns a cell from map at index
spatial_map_cell* get_entry_in_shm(spatial_hash_map* map, matrix_index* index);

//returns the grid position of point within shm
void shm_hash(spatial_hash_map* map, virt_pos* pos, matrix_index* result);

//creates a collider and sets fields based on poly
collider* make_collider_from_polygon(polygon* poly);
//copies the collider
collider* cloneCollider(collider* src);
//frees the collider and collider_ref
void free_collider(collider* rm);
void free_cr(collider_ref* cr);

//creates/frees shm
spatial_hash_map* create_shm(int width, int height, int cols, int rows);
void free_shm(spatial_hash_map* rm);

//creates/frees cells
spatial_map_cell* create_shm_cell();
void free_shm_cell(void* rmv);

//creates/frees index
matrix_index* createMatrixIndex();
void free_matrix_index(matrix_index* rm);

//get the len/width of an shm-cell
box get_cell_shape(spatial_hash_map* shm);
//get the len-width of the entire shm-map
box get_dim_shape(spatial_hash_map* shm);

#endif
