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

polygon* get_polygon(collider* coll);

int update(spatial_hash_map* map, collider* coll, virt_pos* displace, double rot);
void update_refs(collider* coll);

void insert_collider_in_shm(spatial_hash_map* map, collider* collider);
void insert_compound_in_shm(spatial_hash_map* map, struct compound_struct* comp);
void remove_compound_from_shm(spatial_hash_map* map, compound* comp);

void set_cr_vectors(collider* coll, collider_ref* ref, box* cell_dim);

int calc_max_cell_span(double boxW, double boxH, double cellW, double cellH);
void find_bb_for_polygon(polygon* poly, polygon* result);
void fill_bb_dim(polygon* bbox, box* bb_dim);
int get_bb_width (collider* coll);
int get_bb_height (collider* coll);

void recursive_fill(collider* coll, matrix_index ind, vector* result);
void entries_for_collider(spatial_hash_map * map, collider* collider, vector* result);

int matrix_index_difference(matrix_index* m, matrix_index* b);
int adjacent_indexes(matrix_index* m, matrix_index* b);

void remove_collider_from_shm_entries(spatial_hash_map* map, collider_ref* node, vector* entries_to_clear);
void add_collider_to_shm_entries(spatial_hash_map* map, collider_ref* node, vector* entries_to_add);

//results are collider*
int store_unique_colliders_in_list(spatial_hash_map* map, vector* entries, gen_list* result);

void clean_collider_list(gen_list* list);

collider_ref* make_cr_from_collider(collider* coll);

spatial_map_cell* get_entry_in_shm(spatial_hash_map* map, matrix_index* index);

void shm_hash(spatial_hash_map* map, virt_pos* pos, matrix_index* result);


collider* make_collider_from_polygon(polygon* poly);
collider* cloneCollider(collider* src);
void free_collider(collider* rm);

spatial_hash_map* create_shm(int width, int height, int cols, int rows);
void free_shm(spatial_hash_map* rm);

spatial_map_cell* create_shm_cell();
void free_shm_cell(void* rmv);

matrix_index* createMatrixIndex();
void free_matrix_index(matrix_index* rm);

box get_cell_shape(spatial_hash_map* shm);
box get_dim_shape(spatial_hash_map* shm);

#endif
