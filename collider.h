#ifndef FILE_COLLIDER_SEEN
#define FILE_COLLIDER_SEEN

#include "myList.h"
#include "myVector.h"
#include "myMatrix.h"
#include "geometry.h"
#include "physics.h"
#include "physics.h"


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
  //position of center of object
  //virt_pos position;
  //width+height pair would suffice
  //box struct?
  //probably polygon, to resure functions. 
  polygon* shape;
  polygon* bbox;
  box shape_dim;
  //?
  //maybe a type+union combo for owner of collider
  //also collider list node(s) for collider
  struct body_struct* body;
  struct collider_list_node_struct* collider_node;
  
};


struct collider_list_node_struct{
  struct collider_struct* collider;
  //reference stuff
  int max_ref_amount;
  //matrix_index* active_cells;
  //matrix_index* old_cells;
  vector* active_cells;
  vector* old_cells;
  //also, because I'm storing within the map cells a list
  //have to keep some permanant container for the list nodes for collider
  //because I still don't want a movement system that is intensive on mallocs/frees
  
  //gen nodes to put into various shm cells, points to containing collider list node
  gen_node** active_cell_nodes;
  //node to be used in collision resolution, points to collider
  gen_node* cr_node;
  int status;

  //tempted to rename struct something else, like collider references
};

typedef struct collider_struct collider;
typedef struct collider_list_node_struct collider_list_node;


//probably doesn't need to be a struct
//might be nice though, could add fields like an empty flag
//
typedef
struct {
  gen_list * colliders_in_cell;
  
} spatial_map_cell;


typedef
struct {
  //could also be a box struct?
  box cell_dim;
  box matrix_dim;
  gen_matrix* hash_map;
} spatial_hash_map;

int update(spatial_hash_map* map, collider* coll, virt_pos* displace, double rot);

int anyCollisions(spatial_hash_map* map, collider* coll);
  

int safe_move(spatial_hash_map* map, collider* coll, virt_pos* displace);

int safe_rotate(spatial_hash_map* map, collider* coll, double displace);

void update_refs(collider* coll);

int safe_update(spatial_hash_map* map, collider* coll);


void insert_collider_in_shm(spatial_hash_map* map, collider* collider);

collider* make_collider_from_polygon(polygon* poly);

void freeCollider(collider* rm);

void find_bb_for_polygon(polygon* poly, polygon* result);

void entries_for_collider(spatial_hash_map * map, collider* collider, vector* result);

int find_skipped_cell (virt_pos* point1, virt_pos* point2, spatial_hash_map* map,  matrix_index* result);

int matrix_index_difference(matrix_index* m, matrix_index* b);

int adjacent_indexes(matrix_index* m, matrix_index* b);

void remove_collider_from_shm_entries(spatial_hash_map* map, collider_list_node* node, vector* entries_to_clear);

void add_collider_to_shm_entries(spatial_hash_map* map, collider_list_node* node, vector* entries_to_add);

int number_of_unique_colliders_in_entries(spatial_hash_map* map, vector* entries);

int unique_colliders_in_entries(spatial_hash_map* map, vector* entries, collider** results);

int store_unique_colliders_in_list(spatial_hash_map* map, vector* entries, gen_list* result);

void clean_collider_list(gen_list* list);

collider_list_node* make_cln_from_collider(collider* coll);

spatial_map_cell* get_entry_in_shm(spatial_hash_map* map, matrix_index* index);

void shm_hash(spatial_hash_map* map, virt_pos* pos, matrix_index* result);



spatial_hash_map* create_shm(int width, int height, int cols, int rows);

void free_shm(spatial_hash_map* rm);

spatial_map_cell* create_shm_cell();

void free_shm_cell_wrapper(void* rm);

void free_shm_cell(spatial_map_cell* rm);



int check_for_collisions(collider* mono, collider** multi, collider** results);

matrix_index* createMatrixIndex();

//thinking this should go in a very minimalist comparer library
/*
typedef
struct {
  int (*compare)(void* t1, void*t2);
}comparer;
*/
//odd things that shouldbe in a sorting library

/*
int matrix_index_compare(matrix_index* m1, matrix_index* m2);

int matrix_index_unique_insert(gen_list* list, gen_node* data);

int unique_insert(gen_list* list, gen_node* data, comparer* comp);

int matrix_index_unique_insert_array(matrix_index* insert, matrix_index* array);

int collider_unique_insert_array(collider* insert, collider** array);

int unique_insert_array(void* insert, void** array, int fei);
*/

#endif
