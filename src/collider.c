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
    collider_ref* cr = coll->collider_node;
    entries_for_collider(map, coll, cr->old_cells);
    remove_collider_from_shm_entries(map, cr, cr->active_cells);
    add_collider_to_shm_entries(map, cr, cr->old_cells);
    update_refs(coll);
  }
  //fprintf(stderr, "update returns %d\n", change);
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
  if (bbox->sides != 4) {
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

void free_collider(collider* rm) {
  collider_ref* cr = rm->collider_node;
  int size = cr->max_ref_amount;
  for (int i = 0; i < size; i++) {
    remove_node(cr->active_cell_nodes[i]);
  }
  //also need to clear/free vectors from cr
  fprintf(stderr, "Haven't free matrix indexes yet\n");
  free(cr);
  freePolygon(rm->shape);
  //Collider should have center* reassigned to bbox
  if (rm->shape->center != rm->bbox->center) {
    fprintf(stderr, "Some collider didn't get it's center reassigned\nYou will be leaking memory\n");
  }
  rm->bbox->center = NULL;
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
  collider* coll;
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
  collider_ref* node;
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
  
  double rot_threshold = M_PI / 256;
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
    free(result->center);
    result->center = poly->center;
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
  int side_done = 0;
  hash_table* table = collider->collider_node->table;
  clear_table(table);
  matrix_index* temp_ind = NULL;
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

    //take unused element, set
    temp_ind = (matrix_index*)(result->elements[result->cur_size]);
    *temp_ind = curr_ind;
    if (insert(table, temp_ind)) {
      //insert succeded, temp was non_duplicate, increment size to indicate temp is now being used;
      result->cur_size++;
      temp_ind = NULL;
    }
    side_done = 0;
    while(!side_done) {
      double curr_mag, dest_mag;
      curr_mag = get_projected_length_pos(&curr_pos, &dir);
      dest_mag = get_projected_length_pos(&dest_pos, &dir);
      if (matrix_index_difference(&curr_ind, &dest_ind) == 0 ||
	  curr_mag > dest_mag) {
	side_done = 1;
      }
      else {
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
	if (x_corner_off == 0) { //directly on edge, move in y dir
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

	temp_ind = (matrix_index*)(result->elements[result->cur_size]);
	*temp_ind = curr_ind;
	if (insert(table, temp_ind)) {
	  //insert succeded, temp was non_duplicate, increment size to indicate temp is now being used;
	  result->cur_size++;
	  temp_ind = NULL;
	}
      }
    }
    curr_corner = (curr_corner + 1) % bb->sides;
  } while(curr_corner != start_corner);
  
  virt_pos avg = *zero_pos;
  for (int i = 0; i < bb->sides; i++) {
    get_actual_point(bb, i, &curr_pos);
    virt_pos_add(&curr_pos, &avg, &avg);
  }
  avg.x /= bb->sides;
  avg.y /= bb->sides;
  shm_hash(map, &avg, &curr_ind);
  recursive_fill(collider, curr_ind, result);
  set_rotation(bb, orig_rot);
}


void recursive_fill(collider* coll, matrix_index ind, vector* result) {
  hash_table* table = coll->collider_node->table;
  matrix_index* temp = NULL;
  temp = (matrix_index*)(result->elements[result->cur_size]);
  *temp = ind;
  if (insert(table, temp)) {
    result->cur_size++;
    //+x
    ind.x_index += 1;
    recursive_fill(coll, ind, result);
    //-x
    ind.x_index -= 2;
    recursive_fill(coll, ind, result);
    ind.x_index += 1;
    //+y
    ind.y_index += 1;
    recursive_fill(coll, ind, result);
    //-y
    ind.y_index -= 2;
    recursive_fill(coll, ind, result);
  }
}




/*
  iterative solution
  cant merely start at center or corner and go left/right until hitting wall, then ++/-- y ind and reapeat
  or going up/down first then moving left right
  because most rotations/shapes of bounding box will cause outer loop to hit a wall before entire collider is filled
  alternative is starting from two opposite corners of  bounding box, and doing that

  issue with that. , if box has zero rotation and axis are alligned
  then both corners will be visited and incrementing in correct directions also yeild visited nodes
  
  general wonkyness with this means I'll just be using the recursive solution until I can think of a bettwer approach

 



void iterative_fill(spatial_hash_map* map, matrix_index ind, vector* result, int y_sign) {
  matrix_index* temp = NULL;
  hash_table* table = collider->collider_node->table;
  
  int y_done = 0;
  int left_done = 0;
  int right_done = 0;

  int x = 0;
  int y = 0;
  while(!y_done) {
    
    rigth_done = 0;
    left_done = 0;
    x = 0;
    
    while(!left_done || !rigth_done) {
      if (x == 0) {
	//check middle,
	temp = (matrix_index*)(result->elements[result->cur_size]);
	*temp = ind;
	temp->y_index += y * y_sign;
	if (insert(table, temp)) {
	  //new value, start left/right traversals
	  result->cur_size++;
	}
	else {
	  //reached wall, stop trabersal
	  y_done = 1;
	}
      }

      if (x != 0) {
	//check left and right
	temp = (matrix_index*)(result->elements[result->cur_size]);
	*temp = ind;
	temp->x_index += x;
	if (insert(table, temp)) {
	  //new value, keep going right
	  result->cur_size++;
	}
	else {
	  //repeat, stop going right
	  right_done = 1;
	}
	//start checking left
	temp = (matrix_index*)(result->elements[result->cur_size]);
	*temp = ind;
	temp->x_index -= x;
	if (insert(table, temp)) {
	  //new value, keep going left
	  result->cur_size++;
	}
	else {
	  //repeat, stop going left
	  left_done = 1;
	}
      
      }
      x++;
    }

    if (y == 0) {

    }
  }
  
  temp = (matrix_index*)(result->elements[result->cur_size]);
  *temp = curr_ind;
  if (insert(table, temp)) {
    result->cur_size++;
  }
}

*/


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
  new->hash_map = newGen_matrix(width, height);
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
