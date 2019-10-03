#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "myList.h"
#include "myMatrix.h"
//#include "myVector.h"
#include "collider.h"
#include "graphics.h"

int DEFAULT = 0;
int VISITED = 1;
int COUNTED = 2;

int matrix_index_compare(void* a, void* b) {
  matrix_index* m1 = (matrix_index*)a;
  matrix_index* m2 = (matrix_index*)b;
  int result;
  if (m1->y_index < m2->y_index) {
    result = -1;
  }
  else if (m1->y_index > m2->y_index) {
    result = 1;
  }
  else {
    if (m1->x_index < m2->x_index) {
      result = -1;
    }
    else if (m1->x_index > m2->x_index) {
      result = 1;
    }
    else {
      result = 0;
    }
  }
  return result;
}

int matrix_index_hash(void* i) {
  matrix_index* m = (matrix_index*)i;
  
  return m->y_index * m->y_index * 13 + m->x_index * m->x_index * 7;
}



int update(spatial_hash_map* map, collider* coll, virt_pos* displace, double rot) {
  polygon* poly = get_polygon(coll);
  int change = 0;
  if (!isZeroPos(displace)) {
    add_offset_to_center(poly, displace);
    change++;
  }
  if (rot != 0.0) {
    set_rotation(poly, get_rotation(poly) + rot);
    change++;
  }
  if (change > 0) {
    collider_ref* cr = coll->collider_node;
    entries_for_collider(map, coll, cr->old_cells);
    remove_collider_from_shm_entries(map, cr, cr->active_cells);
    add_collider_to_shm_entries(map, cr, cr->old_cells);
    update_refs(coll);
  }
  return change;
}


void update_refs(collider* coll) {
    vector* temp;
    collider_ref* cr = coll->collider_node;
    temp = cr->active_cells;
    cr->active_cells = cr->old_cells;
    cr->old_cells = temp;
}

void fill_bb_dim(polygon* bbox, box* bb_dim) {
  if (get_sides(bbox) != 4) {
    fprintf(stderr, "Trying to get bb_dim of a non-rectangle\n");
  }
  virt_pos c0 = *zero_pos, c1 = *zero_pos, c2 = *zero_pos;
  get_actual_point(bbox, 0, &c0);
  get_actual_point(bbox, 1, &c1);
  get_actual_point(bbox, 2, &c2);
  bb_dim->width = distance_between_points(&c0,&c1);
  bb_dim->height = distance_between_points(&c1,&c2);
}

collider* make_collider_from_polygon(polygon* poly) {
  collider* new = malloc(sizeof(collider));
  new->collider_node = NULL;
  new->shape = poly;
  new->bbox = createPolygon(4);
  find_bb_for_polygon(poly, new->bbox);
  fill_bb_dim(new->bbox, &(new->bb_dim));
  return new;
}

collider* cloneCollider(collider* src) {
  polygon* src_poly = get_polygon(src);
  polygon* p = clonePolygon(src_poly);
  collider* new = make_collider_from_polygon(p);
  return new;
}

polygon* get_polygon(collider* coll) { return coll->shape; }

collider_ref* get_collider_ref(collider* coll) { return coll->collider_node; }

void free_collider(collider* rm) {
  collider_ref* cr = rm->collider_node;
  int size = cr->max_ref_amount;
  for (int i = 0; i < size; i++) {
    remove_node(cr->active_cell_nodes[i]);
  }
  free_cr(cr);
  set_center(rm->bbox, NULL);
  freePolygon(rm->shape);
  freePolygon(rm->bbox);
  free(rm);
}

void free_cr(collider_ref* cr) {
  int size = cr->max_ref_amount;
  //clean & free list nodes
  for (int i = 0; i < size; i++) {
    remove_node(cr->active_cell_nodes[i]);
    freeGen_node(cr->active_cell_nodes[i]);
  }
  //free collider node
  remove_node(cr->cr_node);
  freeGen_node(cr->cr_node);
  
  //then free vectors
  matrix_index* m = NULL;
  for (int i = 0; i < cr->active_cells->max_size; i++) {
    m = elementAt(cr->active_cells, i);
    free_matrix_index(m);
    m = elementAt(cr->old_cells, i);
    free_matrix_index(m);
  }    
  free_vector(cr->active_cells);
  free_vector(cr->old_cells);
  free(cr);
}

void insert_compound_in_shm(spatial_hash_map* map, compound* comp) {
  collider* coll = NULL;
  gen_node* curr = get_bodies(comp)->start;
  while(curr != NULL) {
    coll = ((body*)curr->stored)->coll;
    insert_collider_in_shm(map, coll);
    curr = curr->next;
  }
}

void remove_compound_from_shm(spatial_hash_map* map, compound* comp) {
  gen_node* curr_body = get_bodies(comp)->start;
  collider* coll = NULL;
  collider_ref* cr = NULL;
  while(curr_body != NULL) {
    coll = get_collider((body*)curr_body->stored);
    cr = coll->collider_node;
    remove_collider_from_shm_entries(map, cr, cr->active_cells);
    free_cr(cr);
    coll->collider_node = NULL;
    curr_body = curr_body->next;
  }  
}

void insert_collider_in_shm(spatial_hash_map* map, collider* collider) {
  //setup node reference business
  collider_ref* node = NULL;
  if (collider->collider_node == NULL ) {
    node = make_cr_from_collider(collider);
  }
  else {
    fprintf(stderr, "warning, inserting a collider into spatial hash map when it's already in another\n");
    node = collider->collider_node;
  }
  set_cr_vectors(collider, node, &(map->cell_dim));
 
  //generate active cells for collider
  entries_for_collider(map, collider, node->active_cells);

  //add entries
  add_collider_to_shm_entries(map, node, node->active_cells);
  //done with insertion?
}

void set_cr_vectors(collider* coll, collider_ref* node, box* cell_dim) {
  double cellW, cellH, boxW, boxH;
  cellW = cell_dim->width;
  cellH = cell_dim->height;
  boxW = get_bb_width(coll);
  boxH = get_bb_height(coll);
  int size = calc_max_cell_span(boxW, boxH, cellW, cellH);
  node->max_ref_amount = size;
  node->active_cells = newVectorOfSize(size);
  node->old_cells = newVectorOfSize(size);
  node->active_cell_nodes = calloc(size + 1, sizeof(gen_node));
  node->status = DEFAULT;


  for(int i = 0; i < size; i++) {
    node->active_cell_nodes[i] = createGen_node(node);
    setElementAt(node->active_cells,i, createMatrixIndex());
    setElementAt(node->old_cells,i, createMatrixIndex());
  }
}

int calc_max_cell_span(double boxW, double boxH, double cellW, double cellH) {
  //idea is maximally spanning things are centered in a cell,
  //so extra side lengths grow out on both sides
  //so take box-dim / 2, divide by cell-dim, take ciel, gives # of cells spanned by half of dim
  //do same along other dim, multiply, have cells spanned by quarter of box
  //multiply by 4 to find cells spanned by entire thing
  //since cell size can very and things can rotate, use the min of cellW, cellH
  double cellD = fmin(cellW, cellH);
  int size = ceil((boxW / 2) / cellD + 1) * ceil((boxH / 2) / cellD + 1) * 4 * 1.5;
  /*
  if (size < 4) {
    size = 4;
  }
  */
  return size;
}

void find_bb_for_polygon(polygon* poly, polygon* result) {
  double orig_rot = get_rotation(poly);
  set_rotation(poly, 0);
  double min, max, x_len, y_len;
  double rots[3];
  double areas[3];
  int rot_index;
  rots[0] = 0;
  rots[1] = M_PI / 4;
  double rot_threshold = M_PI / 256;
  virt_pos cent = get_center(poly);
  while(rots[1] - rots[0] > rot_threshold) {
    rots[2] = (rots[1] + rots[0] )/ 2;
    for (int i = 0; i < 3; i++) {
      set_rotation(poly, rots[i]);
      extreme_projections_of_polygon(poly, &cent, x_axis, &min, &max);
      x_len = fabs(max - min);
      extreme_projections_of_polygon(poly, &cent, y_axis, &min, &max);
      y_len = fabs(max - min);
      areas[i] = y_len * x_len;
    }
    if (areas[0] < areas[1]) {
      rots[1] = rots[2];
      rot_index = 0;
    }
    else {
      rots[0] = rots[2];
      rot_index = 1;
    }
  }

  double xmin, xmax, ymin, ymax;
  if (get_sides(result) == 4) {
    set_rotation(result,rots[rot_index]);
    extreme_projections_of_polygon(poly, &cent, x_axis, &xmin, &xmax);
    extreme_projections_of_polygon(poly, &cent, y_axis, &ymin, &ymax);
    set_base_point(result, 0, &(virt_pos){.x = xmin, .y = ymin});
    set_base_point(result, 1, &(virt_pos){.x = xmax, .y = ymin});
    set_base_point(result, 2, &(virt_pos){.x = xmax, .y = ymax});
    set_base_point(result, 3, &(virt_pos){.x = xmin, .y = ymax});
    generate_normals_for_polygon(result);
    free_polygon_center(result);
    set_center_p(result, read_only_polygon_center(poly));
  }
  else {
    fprintf(stderr, "handed in a not 4 sided thing to find_bound_box\n");
  }
  set_rotation(poly, orig_rot);

}


int get_bb_width (collider* coll) {
  return coll->bb_dim.width;
}

int get_bb_height (collider* coll) {
  return coll->bb_dim.height;
}

void entries_for_polygon(spatial_hash_map * map, polygon* p, hash_table* table, vector* result) {
  int cellW = map->cell_dim.width;
  int cellH = map->cell_dim.height;
  int dest_corner;
  int sides = get_sides(p);
  virt_pos  curr_pos, dest_pos, avg;

  matrix_index curr_ind, dest_ind;

  int x_corner_off, y_corner_off;
  int x_ind_off, y_ind_off;
  int side_done = 0;
  
  result->cur_size = 0;
  clear_table(table);

  matrix_index* temp_ind = NULL;

  virt_pos offset_to_dest = *zero_pos;
  virt_pos offset_step = *zero_pos;
  double step_size = 0;
  for (int curr_corner = 0; curr_corner < sides; curr_corner++) {
    dest_corner = (curr_corner + 1) % sides;
    get_actual_point(p, curr_corner, &curr_pos);
    get_actual_point(p, dest_corner, &dest_pos);

    shm_hash(map, &dest_pos, &dest_ind);
    shm_hash(map, &curr_pos, &curr_ind);
    
    temp_ind = (matrix_index*)(result->elements[result->cur_size]);
    *temp_ind = curr_ind;
    if (insert(table, temp_ind)) {
      result->cur_size++;
      temp_ind = NULL;
    }
    
    side_done = 0;
    while(!side_done) {
      shm_hash(map, &curr_pos, &curr_ind);
      virt_pos_sub(&dest_pos, &curr_pos, &offset_to_dest);
      x_ind_off = sign_of(offset_to_dest.x);
      y_ind_off = sign_of(offset_to_dest.y);
      
      if (matrix_index_difference(&curr_ind, &dest_ind) == 0) {
	side_done = 1;
      }
      else {
	x_corner_off = (curr_ind.x_index + 1) * cellW - curr_pos.x;
	if (x_ind_off < 0) {
	  x_corner_off = cellW - x_corner_off + 1;
	}
	y_corner_off = (curr_ind.y_index + 1) * cellH - curr_pos.y;
	if (y_ind_off < 0) {
	  y_corner_off = cellH - y_corner_off + 1;
	}

	if (x_ind_off == 0) {
	  step_size = 1.0 * y_corner_off / offset_to_dest.y;
	}
	else if (y_ind_off == 0) {
	  step_size = 1.0 * x_corner_off / offset_to_dest.x;
	}
	else if (fabs((float)y_corner_off / x_corner_off) < fabs((float)offset_to_dest.y / offset_to_dest.x)) {
	  //offset to dests slope is higher greater than corner, will change y index
	  step_size = 1.0 * y_corner_off / offset_to_dest.y;
	}
	else {
	  //change x index
	  step_size = 1.0 * x_corner_off / offset_to_dest.x;
	}
	step_size = fabs(step_size);
	virt_pos_scale(&offset_to_dest, step_size, &offset_step);
	virt_pos_add(&curr_pos, &offset_step, &curr_pos);
	
	shm_hash(map, &curr_pos, &curr_ind);

	temp_ind = (matrix_index*)(result->elements[result->cur_size]);
	*temp_ind = curr_ind;
	if (insert(table, temp_ind)) {
	  result->cur_size++;
	  temp_ind = NULL;
	}
      }
    }
  }

  avg = *zero_pos;
  for (int i = 0; i < sides; i++) {
    get_actual_point(p, i, &curr_pos);
    virt_pos_add(&curr_pos, &avg, &avg);
  }
  avg.x /= sides;
  avg.y /= sides;
  shm_hash(map, &avg, &curr_ind);
  recursive_fill(table, curr_ind,  result);
}


void entries_for_collider(spatial_hash_map * map, collider* collider, vector* result) {

  polygon* p = collider->shape;
  polygon* bb = collider->bbox;
  double orig_rot = get_rotation(bb);
  set_rotation(bb, get_rotation(bb) + get_rotation(p));
  //code also works for p, so I could get a better fitting set of entries than with bounding box
  entries_for_polygon(map, bb, collider->collider_node->table, result);
  set_rotation(bb, orig_rot);
}


void recursive_fill( hash_table* table, matrix_index ind ,vector* result) {
  matrix_index* temp = NULL;
  temp = (matrix_index*)(result->elements[result->cur_size]);
  *temp = ind;
  if (insert(table, temp)) {
    result->cur_size++;
    //+x
    ind.x_index += 1;
    recursive_fill(table, ind, result);
    //-x
    ind.x_index -= 2;
    recursive_fill(table, ind, result);
    ind.x_index += 1;
    //+y
    ind.y_index += 1;
    recursive_fill(table, ind, result);
    //-y
    ind.y_index -= 2;
    recursive_fill(table, ind, result);
  }
}

int matrix_index_difference(matrix_index* m, matrix_index* b) {
  return abs(m->x_index - b->x_index) + abs(m->y_index - b->y_index);
}

int adjacent_indexes(matrix_index* m, matrix_index* b) {
  if (matrix_index_difference(m,b) == 1) {
    return 1;
  }
  else {
    return 0;
  }
}

void remove_collider_from_shm_entries(spatial_hash_map* map, collider_ref* node, vector* entries_to_clear) {
  int i = 0;
  spatial_map_cell* cell;
  gen_node* list_node;
  while(i < entries_to_clear->cur_size) {
    cell = get_entry_in_shm(map, elementAt(entries_to_clear,i));
    if (cell  != NULL) {
      list_node = node->active_cell_nodes[i];
      removeFromGen_list(cell->colliders_in_cell, list_node);
    }
    i++;
  }
}

void add_collider_to_shm_entries(spatial_hash_map* map, collider_ref* node, vector* entries_to_add) {
  int i = 0;
  spatial_map_cell* cell;
  gen_node* list_node;
  matrix_index* ind;
  while(i < entries_to_add->cur_size) {
    ind = elementAt(entries_to_add,i);
    cell = get_entry_in_shm(map, ind);
    if (cell  != NULL) {
      list_node = node->active_cell_nodes[i];
      remove_node(list_node);
      prependToGen_list(cell->colliders_in_cell, list_node);
    }
    i++;
  }
}

int store_unique_colliders_in_list(spatial_hash_map* map, vector* entries, gen_list* result) {
  spatial_map_cell* cell;
  gen_node* curr;
  gen_node* collider_cr_node;
  collider_ref* cr;
  for(int i = 0; i < entries->cur_size;i++) {
    cell = get_entry_in_shm(map, elementAt(entries, i));
    if (cell != NULL) {
      curr = cell->colliders_in_cell->start;
      while (curr != NULL) {
	cr = (collider_ref*)(curr->stored);
	if (cr->status == DEFAULT) {
	  cr->status = VISITED;
	  collider_cr_node = cr->cr_node;
	  /*
	  if (already_in_a_list(collider_cr_node)) {
	    //print things
	    assert(0 && "Have old list references, stop that");
	  }
	  */
	  prependToGen_list(result, collider_cr_node);
	}
	curr = curr->next;
      }
    }
  }
  return 0;
}

void clean_collider_list(gen_list* list) {
  gen_node* curr= list->start;
  gen_node* collider_cr_node;
  collider_ref* cr;
  while (curr != NULL) {
    cr = ((collider*)(curr->stored))->collider_node;
    cr->status = DEFAULT;
    collider_cr_node = curr;
    curr = curr->next;
    remove_node(collider_cr_node);
  }
}

collider_ref* make_cr_from_collider(collider* coll) {
  collider_ref* new = malloc(sizeof(collider_ref));
  new->collider = coll;
  new->status = DEFAULT;
  new->cr_node = createGen_node(coll);
  coll->collider_node = new;
  new->table = create_hash_table( matrix_index_compare, matrix_index_hash, 0);
  return new;
}

spatial_map_cell* get_entry_in_shm(spatial_hash_map* map, matrix_index* index) {
  void* data = getDataAtIndex(map->hash_map, index->x_index, index->y_index);
  return (spatial_map_cell*)data;
}
void shm_hash(spatial_hash_map* map, virt_pos* pos, matrix_index* result) {
  //draw_virt_pos(getCam(), pos);
  int x_index = (int)floor((double)pos->x / map->cell_dim.width);
  int y_index = (int)floor((double)pos->y / map->cell_dim.height);
  result->x_index = x_index;
  result->y_index = y_index;
}



spatial_hash_map* create_shm(int width, int height, int cols, int rows) {
  spatial_hash_map* new = malloc(sizeof(spatial_hash_map));
  new->cell_dim.width = width;
  new->cell_dim.height = height;
  new->matrix_dim.width = cols;
  new->matrix_dim.height = rows;
  new->hash_map = newGen_matrix(cols, rows);
  for(int col_i = 0; col_i < cols; col_i++) {
    for(int row_i = 0; row_i < rows; row_i++) {
      setDataAtIndex(new->hash_map, col_i, row_i, create_shm_cell());
    }
  }
  new->hash_map->free_data_function = free_shm_cell;
  return new;
  
}

void free_shm(spatial_hash_map* rm) {
  freeGen_matrix(rm->hash_map);
  free(rm);
}

spatial_map_cell* create_shm_cell() {
  spatial_map_cell* new = malloc(sizeof(spatial_map_cell));
  new->colliders_in_cell = createGen_list();
  return new;
}

void free_shm_cell(void* rmv) {
  
  //can free colliders within freeNpc or other freeObject functions
  //this just needs to clean up the gen_list for now.
  spatial_map_cell* rm = (spatial_map_cell*)rmv;
  freeGen_list(rm->colliders_in_cell);
  free(rm);
}


matrix_index* createMatrixIndex() {
  matrix_index* new = malloc(sizeof(new));
  new->x_index = 0;
  new->y_index = 0;
  return new;
}

 void free_matrix_index(matrix_index* rm) {
   free(rm);
}

box get_cell_shape(spatial_hash_map* shm) {
  return shm->cell_dim;
}

box get_dim_shape(spatial_hash_map* shm) {
  return shm->matrix_dim;
}
