#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "myList.h"
#include "myMatrix.h"
#include "myVector.h"
#include "collider.h"
#include "graphics.h"

//comparer stuff
int matrix_index_compare(matrix_index* m1, matrix_index* m2) {
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

int matrix_index_compare_wrapper(void* m, void* b) {
  return matrix_index_compare((matrix_index*)m,(matrix_index*)b);
}




comparer matrix_index_comp = {.compare= matrix_index_compare_wrapper};




int update(spatial_hash_map* map, collider* coll, virt_pos* displace, double rot) {
  polygon* poly = coll->shape;
  int change = 0;
  //doing this to constnatly display shm hashes
  change = 0;
  if (!isZeroPos(displace)) {
    virt_pos_add(poly->center, displace, poly->center);
    change++;
  }
  if (rot != 0.0) {
    set_rotation(poly, get_rotation(poly) + rot);
    //poly->rotation += rot;
    change++;
  }
  if (change > 0) {
    collider_list_node* cln = coll->collider_node;
    entries_for_collider(map, coll, cln->old_cells);
    remove_collider_from_shm_entries(map, cln, cln->active_cells);
    add_collider_to_shm_entries(map, cln, cln->old_cells);
    update_refs(coll);
  }
  //fprintf(stderr, "update returns %d\n", change);
  return change;
}



void update_refs(collider* coll) {
    vector* temp;
    collider_list_node* cln = coll->collider_node;
    temp = cln->active_cells;
    cln->active_cells = cln->old_cells;
    cln->old_cells = temp;
}



collider* make_collider_from_polygon(polygon* poly) {
  collider* new = malloc(sizeof(collider));
  new->collider_node = NULL;
  new->shape = poly;
  new->bbox = createPolygon(4);
  find_bb_for_polygon(poly, new->bbox);
  return new;
}

polygon* get_polygon(collider* coll) { return coll->shape; }

void free_collider(collider* rm) {
  collider_list_node* cln = rm->collider_node;
  int size = cln->max_ref_amount;
  for (int i = 0; i < size; i++) {
    remove_node(cln->active_cell_nodes[i]);
  }
  //also need to clear/free vectors from cln
  fprintf(stderr, "Haven't free matrix indexes yet\n");
  free(cln);
  freePolygon(rm->shape);
  //Collider should have center* reassigned to bbox
  if (rm->shape->center != rm->bbox->center) {
    fprintf(stderr, "Some collider didn't get it's center reassigned\nYou will be leaking memory\n");
  }
  rm->bbox->center = NULL;
  freePolygon(rm->bbox);
  free(rm);
}

void insert_compound_in_shm(spatial_hash_map* map, compound* comp) {
  collider* coll;
  gen_node* curr = comp->bp->start;
  while(curr != NULL) {
    coll = ((body*)curr->stored)->coll;
    insert_collider_in_shm(map, coll);
    curr = curr->next;
  }
}

void insert_collider_in_shm(spatial_hash_map* map, collider* collider) {
  //setup node reference business
  collider_list_node* node;
  if (collider->collider_node == NULL ) {
    node = make_cln_from_collider(collider);
  }
  else {
    fprintf(stderr, "warning, inserting a collider into spatial hash map when it's already in another\n");
    node = collider->collider_node;
  }
  set_cln_vectors(collider, node, &(map->cell_dim));
 
  //generate active cells for collider
  entries_for_collider(map, collider, node->active_cells);

  //add entries
  add_collider_to_shm_entries(map, node, node->active_cells);
  //done with insertion?
}

void set_cln_vectors(collider* coll, collider_list_node* node, box* cell_dim) {
  polygon* bbox = coll->bbox;
  double cellW, cellH, boxW, boxH;
  cellW = cell_dim->width;
  cellH = cell_dim->height;
  //potentially need to factor in scale in here, though I don't think i use scale in construction bounding boxes
  virt_pos c0 = *zero_pos, c1 = *zero_pos, c2 = *zero_pos;
  get_actual_point(bbox, 0, &c0);
  get_actual_point(bbox, 1, &c1);
  get_actual_point(bbox, 2, &c2);
  boxW = distance_between_points(&c0,&c1);
  boxH = distance_between_points(&c1,&c2); 
  int size = calc_max_cell_span(boxW, boxH, cellW, cellH);
  node->max_ref_amount = size;
  node->active_cells = newVectorOfSize(size);
  node->old_cells = newVectorOfSize(size);
  node->active_cell_nodes = calloc(size + 1, sizeof(gen_node));
  node->status = 0;


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
  //in actuality, there's a bit of a kink since in  the max_spanning setup
  //the box dim only has to be greater than half the cell dim to clear the first cell
  //after that though the box dim has to grow by full cell dim amount to clear later cells
  //I could either work some weird math to account for that, or just add 1
  int size = ceil(boxW / 2 / cellW + 1) * ceil(boxH / 2 / cellH + 1) * 4;
  if (size < 4) {
    size = 4;
  }
  return size;
}

void find_bb_for_polygon(polygon* poly, polygon* result) {
  double orig_rot = get_rotation(poly);
  set_rotation(poly, 0);
  double min, max, x_len, y_len;
  double rots[3];
  double areas[3];
  int rot_index;
  vector_2 x_axis = (vector_2){.v1 = 1, .v2 = 0}, y_axis = (vector_2){.v1 = 0, .v2 = 1};
  rots[0] = 0;
  rots[1] = M_PI / 4;
  
  double rot_threshold = M_PI / 16;
  while(rots[1] - rots[0] > rot_threshold) {
    rots[2] = (rots[1] + rots[0] )/ 2;
    for (int i = 0; i < 3; i++) {
      set_rotation(poly, rots[i]);
      extreme_projections_of_polygon(poly, poly->center, &x_axis, &min, &max);
      x_len = fabs(max - min);
      extreme_projections_of_polygon(poly, poly->center, &y_axis, &min, &max);
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
  //now, create polygon
  //make square, set rotation to rots[2]
  //then set it to the  xlen and ylen projections
  //manually set points to (vir_pos){.x = +- x_len / 2, .y = +- y_len / 2};
  double xmin, xmax, ymin, ymax;
  if (result->sides == 4) {
    set_rotation(result,rots[rot_index]);
    extreme_projections_of_polygon(poly, poly->center, &x_axis, &xmin, &xmax);
    extreme_projections_of_polygon(poly, poly->center, &y_axis, &ymin, &ymax);
    set_base_point(result, 0, &(virt_pos){.x = xmin, .y = ymin});
    set_base_point(result, 1, &(virt_pos){.x = xmax, .y = ymin});
    set_base_point(result, 2, &(virt_pos){.x = xmax, .y = ymax});
    set_base_point(result, 3, &(virt_pos){.x = xmin, .y = ymax});
    generate_normals_for_polygon(result);
    //then, need to free results center, and point it to poly's center
    free(result->center);
    result->center = poly->center;
  }
  else {
    fprintf(stderr, "handed in a not 4 sided thing to find_bound_box\n");
  }
  set_rotation(poly, orig_rot);

}


/*
  take bounding box, traverse all 4 sides, add/fill in hashed indicies
  then, simply fill the interior
  
  filling outline of bbox
  multiple ways of doing this
  easist would actually be just taking shm hashes of curr and dest from each corner
  and just kind of incrementing/decrementing indeses by one until complete
  take corners of current cell, represent in coordinate space with curr pos as origin
  check which quadrant line from cur to dest goes through by signs of x/y
  compare slope of line to slope of point in quadrant. can tell which cell you end up in next
  */


void entries_for_collider(spatial_hash_map * map, collider* collider, vector* result) {
  int cellW = map->cell_dim.width;
  int cellH = map->cell_dim.height;
  int cell_index = 0;
  int start_corner = 0;
  int curr_corner = 0;
  int dest_corner;
  polygon* bb = collider->bbox;
  double orig_rot = get_rotation(bb);
  set_rotation(bb, get_rotation(bb) + get_rotation(collider->shape));
  virt_pos  curr_pos, dest_pos, temp;
  vector_2 dir, unit;
  matrix_index curr_ind, dest_ind;

  int x_corner_off, y_corner_off;
  vector_2 x_unit, y_unit, disp;
  int x_ind_off, y_ind_off;
  double small_amount = 0.001;
  result->cur_size = 0;
  do {
    dest_corner = (curr_corner + 1) % bb->sides;
    get_actual_point(bb, curr_corner, &curr_pos);
    get_actual_point(bb, dest_corner, &dest_pos);
    vector_between_points(&curr_pos, &dest_pos, &dir);
    make_unit_vector(&dir, &unit);

    x_ind_off = 1;
    y_ind_off = 1;
    if (dir.v1 < 0) {
      x_ind_off = -1;
    }
    if (dir.v2 < 0) {
      y_ind_off = -1;
    }
    x_unit = unit;
    y_unit = unit;
    vector_2_scale(&x_unit, fabs(1.0 / x_unit.v1), &x_unit);
    vector_2_scale(&y_unit, fabs(1.0 / y_unit.v2), &y_unit);
    
    
    shm_hash(map, &curr_pos, &curr_ind);
    shm_hash(map, &dest_pos, &dest_ind);
    
    while(matrix_index_difference(&curr_ind, &dest_ind) > 0) {
      //just calculating offest of corner relative to curr position
      x_corner_off = (curr_ind.x_index + 1) * cellW - curr_pos.x;
      if (x_ind_off < 0) {
	x_corner_off = cellW - x_corner_off + 1;
      }
      y_corner_off = (curr_ind.y_index + 1) * cellH - curr_pos.y;
      if (y_ind_off < 0) {
	y_corner_off = cellH - y_corner_off + 1;
      }
      //checks for edge cases then compares slopes
      if (x_corner_off == 0) { //directly on edge, move in y dir I guess?
	vector_2_scale(&x_unit, x_corner_off + small_amount, &disp);	
      }
      else if (dir.v1 == 0) { //moving purely up, move in y dir
	vector_2_scale(&y_unit, y_corner_off + small_amount, &disp);
      }

      else if (x_corner_off > 0 && fabs((float)y_corner_off / x_corner_off) < fabs((float)dir.v2 / dir.v1)) {
	vector_2_scale(&y_unit, y_corner_off + small_amount, &disp);
      }
      else {
	vector_2_scale(&x_unit, x_corner_off + small_amount, &disp);	
      }
      vector_2_to_virt_pos(&disp, &temp);
      virt_pos_add(&curr_pos, &temp, &curr_pos);
      shm_hash(map, &curr_pos, &curr_ind);
      
      if (!already_in_vector(result, &curr_ind, &matrix_index_comp)) {
	*(matrix_index*)(result->elements[result->cur_size]) = curr_ind;
	result->cur_size++;
      }
    }
    curr_corner = (curr_corner + 1) % bb->sides; ;
  } while(curr_corner != start_corner);
  //that filled the outline, fill interior by taking average of corners
  int x_avg = 0, y_avg = 0;
  virt_pos avg = *zero_pos;
  for (int i = 0; i < bb->sides; i++) {
    get_actual_point(bb, i, &curr_pos);
    virt_pos_add(&curr_pos, &avg, &avg);
  }
  avg.x /= bb->sides;
  avg.y /= bb->sides;
  shm_hash(map, &avg, &curr_ind);
  //start a recursive fill, handing in map, ind, and vector of results I guess?
  recursive_fill(map, curr_ind, result);
  set_rotation(bb, orig_rot);
}


void recursive_fill(spatial_hash_map* map, matrix_index ind, vector* result) {
  if (!already_in_vector(result, &ind, &matrix_index_comp)) {
    *(matrix_index*)(result->elements[result->cur_size]) = ind;
    result->cur_size++;
    //+x
    ind.x_index += 1;
    recursive_fill(map, ind, result);
    //-x
    ind.x_index -= 2;
    recursive_fill(map, ind, result);
    ind.x_index += 1;
    //+y
    ind.y_index += 1;
    recursive_fill(map, ind, result);
    //-y
    ind.y_index -= 2;
    recursive_fill(map, ind, result);
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

void remove_collider_from_shm_entries(spatial_hash_map* map, collider_list_node* node, vector* entries_to_clear) {
  int i = 0;
  spatial_map_cell* cell;
  gen_node* list_node;
  //again, use vectors!@
  while(i < entries_to_clear->cur_size) {
    cell = get_entry_in_shm(map, elementAt(entries_to_clear,i));
    if (cell  != NULL) {
      list_node = node->active_cell_nodes[i];
      removeFromGen_list(cell->colliders_in_cell, list_node);
    }
    i++;
  }
}

void add_collider_to_shm_entries(spatial_hash_map* map, collider_list_node* node, vector* entries_to_add) {
  int i = 0;
  spatial_map_cell* cell;
  gen_node* list_node;
  matrix_index* ind;
  while(i < entries_to_add->cur_size) {
    ind = elementAt(entries_to_add,i);
    cell = get_entry_in_shm(map, ind);
    if (cell  != NULL) {
      list_node = node->active_cell_nodes[i];
      prependToGen_list(cell->colliders_in_cell, list_node);
    }
    i++;
  }
}


int DEFAULT = 0;
int VISITED = 1;
int COUNTED = 2;

int number_of_unique_colliders_in_entries(spatial_hash_map* map, vector* entries) {
  int count = 0;
  spatial_map_cell* cell;
  gen_node* curr ;
  collider_list_node* cln;
  for(int i = 0; i < entries->cur_size; i++) {
    cell = get_entry_in_shm(map, elementAt(entries,i));
    if (cell != NULL) {
      curr = cell->colliders_in_cell->start;
      while(curr != NULL) {
	cln = (collider_list_node*)(curr->stored);
	if (cln->status == DEFAULT) {
	  cln->status = VISITED;
	  count++;
	}
	curr = curr->next;
      }
    }
  }
  return count;
}

int unique_colliders_in_entries(spatial_hash_map* map, vector* entries, collider** results) {
  int count = 0;
  spatial_map_cell* cell;
  gen_node* curr ;
  collider_list_node* cln;
  for(int i = 0; i < entries->cur_size; i++) {
    cell = get_entry_in_shm(map, elementAt(entries,i));
    if (cell != NULL) {
      curr = cell->colliders_in_cell->start;
      while(curr != NULL) {
	cln = (collider_list_node*)(curr->stored);
	if (cln->status == VISITED) {
	  cln->status = DEFAULT;
	  results[count] = cln->collider;
	  count++;
	}
	curr = curr->next;
      }
    }
  }
  return count;
}

int store_unique_colliders_in_list(spatial_hash_map* map, vector* entries, gen_list* result) {
  spatial_map_cell* cell;
  gen_node* curr;
  gen_node* collider_cr_node;
  collider_list_node* cln;
  for(int i = 0; i < entries->cur_size;i++) {
    cell = get_entry_in_shm(map, elementAt(entries, i));
    if (cell != NULL) {
      curr = cell->colliders_in_cell->start;
      while (curr != NULL) {
	cln = (collider_list_node*)(curr->stored);
	if (cln->status == DEFAULT) {
	  cln->status = VISITED;
	  collider_cr_node = cln->cr_node;
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
  collider_list_node* cln;
  while (curr != NULL) {
    cln = ((collider*)(curr->stored))->collider_node;
    cln->status = DEFAULT;
    collider_cr_node = curr;
    curr = curr->next;
    remove_node(collider_cr_node);
  }
}

collider_list_node* make_cln_from_collider(collider* coll) {
  collider_list_node* new = malloc(sizeof(collider_list_node));
  new->collider = coll;
  new->status = DEFAULT;
  new->cr_node = createGen_node(coll);
  coll->collider_node = new;
  return new;
}

spatial_map_cell* get_entry_in_shm(spatial_hash_map* map, matrix_index* index) {
  void* data = getDataAtIndex(map->hash_map, index->x_index, index->y_index);
  return (spatial_map_cell*)data;
}
void shm_hash(spatial_hash_map* map, virt_pos* pos, matrix_index* result) {
  draw_virt_pos(getCam(), pos);
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
  new->hash_map = newGen_matrix(width, height);
  for(int col_i = 0; col_i < cols; col_i++) {
    for(int row_i = 0; row_i < rows; row_i++) {
      setDataAtIndex(new->hash_map, col_i, row_i, create_shm_cell());
    }
  }
  new->hash_map->free_data_function = free_shm_cell_wrapper;
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

void free_shm_cell_wrapper(void* rm) {
  free_shm_cell((spatial_map_cell*)rm);
}

void free_shm_cell(spatial_map_cell* rm) {
  //can free colliders within freeNpc or other freeObject functions
  //this just needs to clean up the gen_list for now.
  freeGen_list(rm->colliders_in_cell);
  free(rm);
}


matrix_index* createMatrixIndex() {
  matrix_index* new = malloc(sizeof(new));
  new->x_index = 0;
  new->y_index = 0;
  return new;
}
