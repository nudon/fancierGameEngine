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
  if (!isZeroPos(displace)) {
    virt_pos_add(&(poly->center), displace, &(poly->center));
    change++;
  }
  if (rot != 0.0) {
    poly->rotation += rot;
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


//safe variants of above functions
//doesn't carry out updates if it resulted in collision
//moves each object indiivdually and reverts if bad
int safe_move(spatial_hash_map* map, collider* coll, virt_pos* displace) {
  virt_pos old_pos = coll->shape->center;
  polygon* poly = coll->shape;
  virt_pos_add(&(poly->center), displace, &(poly->center));
  int safe = safe_update(map, coll);
  if ( !safe) {
    poly->center = old_pos;
  }
  else {
    update_refs(coll);
  }
  return safe;
}

int safe_rotate(spatial_hash_map* map, collider* coll, double displace) {
  double old_rot = coll->shape->rotation;
  polygon* poly = coll->shape;
  poly->rotation += displace;
  int safe = safe_update(map, coll);
  if (!safe) {
    poly->rotation = old_rot;
  }
  else {
    update_refs(coll);
  }
  return safe;
}

void update_refs(collider* coll) {
    vector* temp;
    collider_list_node* cln = coll->collider_node;
    temp = cln->active_cells;
    cln->active_cells = cln->old_cells;
    cln->old_cells = temp;
}

int safe_update(spatial_hash_map* map, collider* coll){
  //returns 1 if update is safe, 0 if not
  polygon* poly = coll->shape;
  int size, ret = 1;
  collider_list_node* cln = coll->collider_node;
  entries_for_collider(map, coll, cln->old_cells);
  remove_collider_from_shm_entries(map, cln, cln->active_cells);
  
  add_collider_to_shm_entries(map, cln, cln->old_cells);
  size = number_of_unique_colliders_in_entries(map, cln->old_cells);
  collider* entries[size];
  size = unique_colliders_in_entries(map, cln->old_cells, entries);
  collider* curr;
  for (int i = 0; i < size; i++) {
    curr = entries[i];
    //just need to check if curr is the same as coll
    if (curr != coll) {
      if (do_polygons_intersect(poly, curr->shape)) {
	//break, revert movement
	ret = 0;
	break;
      }
    }
  }
  if (ret == 0) {
    remove_collider_from_shm_entries(map, cln, cln->old_cells);
    
    add_collider_to_shm_entries(map, cln, cln->active_cells);
  }
  return ret;
}

int anyCollisions(spatial_hash_map* map, collider* coll) {
  //returns 1 if there are collisions, 0 if none
  polygon* poly = coll->shape;
  int size, ret = 0;
  collider_list_node* cln = coll->collider_node;
  vector* currCells = cln->active_cells;
  entries_for_collider(map, coll, currCells);
  size = number_of_unique_colliders_in_entries(map, currCells);
  collider* entries[size];
  size = unique_colliders_in_entries(map, currCells, entries);
  collider* curr;
  //fprintf(stderr, "There are %d colliders to check against for collisions!\n", size);
  for (int i = 0; i < size; i++) {
    curr = entries[i];
    if (curr != coll) {
      if (do_polygons_intersect(poly, curr->shape)) {
	ret = 1;
	break;
      }
    }
  }
  return ret;
}



collider* make_collider_from_polygon(polygon* poly) {
  collider* new = malloc(sizeof(collider));
  new->collider_node = NULL;
  new->shape = poly;
  new->bbox = createPolygon(4);
  find_bb_for_polygon(poly, new->bbox);
  return new;
}

void freeCollider(collider* rm) {
  collider_list_node* cln = rm->collider_node;
  int size = cln->max_ref_amount;
  for (int i = 0; i < size; i++) {
    remove_node(cln->active_cell_nodes[i]);
  }
  //also need to clear/free vectors from cln
  fprintf(stderr, "Haven't free matrix indexes yet\n");
  free(cln);
  freePolygon(rm->shape);
  freePolygon(rm->bbox);
  free(rm);
}

void insert_collider_in_shm(spatial_hash_map* map, collider* collider) {
  //setup node reference business
  collider_list_node* node;
  if (collider->collider_node == NULL ) {
    node = make_cln_from_collider(collider);
    collider->collider_node = node;
  }
  else {
    fprintf(stderr, "warning, inserting a collider into spatial hash map when it's already in another\n");
    node = collider->collider_node;
  }


  
  polygon* bbox = collider->bbox;
  double cellW, cellH, boxW, boxH;
  cellW = map->cell_dim.width;
  cellH = map->cell_dim.height;
  //potentially need to factor in scale in here, though I don't think i use scale in construction bounding boxes
  boxW = distance_between_points(&(bbox->corners[0]),&(bbox->corners[1]));
  boxH = distance_between_points(&(bbox->corners[1]),&(bbox->corners[2])); 
  //need to calculate bounding box of collider.
  //was thinking of just doing a naive iteration
  //take polygon, project along 2 orth axis, get difference between min/max projections, get area of box
  //then change rotation so that area decreases. keep changing that way until area stops decreaseing
  //or if I want to lazy, don't even optimize, just take box you calculate.
  int area_span;
  int perimeter_span;
  //size is off, consider box w/ respective dim 1.1 bigger than cell width
  //could span 9 boxes, but the bellow calc would give like 4
  //area_span = ceil((boxW * boxH) / (cellW * cellH));
  //perimeter_span  = ceil((boxW + boxH) / (cellW + cellH));
  //instead, ? on area
  //for perim, have  s = (box_dim / 2) /cell_dim
  //some odd exact breakdowns, but span(box_dim) = ceil(s) * 4
  //idea is maximally spanning things are centered along cell dim,
  //so extra side lengths grow out on both sides
  //so take dim / 2, find ceil of that, * 2 to grab whole side, * 2 for other size
  //do same for other dim, perimiter span should be product of other spans
  area_span = ceil((boxW * boxH) / (cellW * cellH));
  perimeter_span = (ceil((boxW / 2) / cellW) + ceil(boxH / 2) / cellH) * 4;
  int size = area_span + perimeter_span;
  if (size < 4) {
    size = 4;
  }
  
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
  
  //generate active cells for collider
  entries_for_collider(map, collider, node->active_cells);

  //add entries
  add_collider_to_shm_entries(map, node, node->active_cells);
  //done with insertion?
}

void find_bb_for_polygon(polygon* poly, polygon* result) {
  //another option, need to somehow tie center of polygon with center of bound box
  //could either set centre point to same as poly
  //if so, would have to change how I'm defining bb_corners, as those assume bb_center is geometric center of bbox, which if it's shared with some polygon, it might not be
  //for finding corners of box relative to corner, could probably easily find these from calling extreme_projections_of_polygon on center of poly/result. the min/max points should give components of corner positions

  //alternative is to simultanteously update both poly and it's bound box. that's annoying

  //actually issue is slightly more annoying because center is not a pointer
  //might just copy select polygon values when needing to do stuff with bound box

  //was also thinking, if stacking rotation between boundbox and poly is an issue, could make rotation in poly point to rot in poly
  //would take points at end of this and rotate by the optimal rotation

  //currently, only dealing with bounding box in entries for collider. so it wouldn't be too unwieldy to just manually copy center and rotations
  
  double orig_rot = poly->rotation;
  poly->rotation = 0;
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
      poly->rotation = rots[i];
      extreme_projections_of_polygon(poly, &(poly->center), &x_axis, &min, &max);
      x_len = fabs(max - min);
      extreme_projections_of_polygon(poly, &(poly->center), &y_axis, &min, &max);
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
  //then set it to the now lost xlen and ylen projections
  //manually set points to (vir_pos){.x = +- x_len / 2, .y = +- y_len / 2};
  double xmin, xmax, ymin, ymax;
  if (result->sides == 4) {
    result->rotation = rots[rot_index];
    extreme_projections_of_polygon(poly, &(poly->center), &x_axis, &xmin, &xmax);
    extreme_projections_of_polygon(poly, &(poly->center), &y_axis, &ymin, &ymax);
    result->corners[0] = (virt_pos){.x = xmin, .y = ymin};
    result->corners[1] = (virt_pos){.x = xmax, .y = ymin};
    result->corners[2] = (virt_pos){.x = xmax, .y = ymax};
    result->corners[3] = (virt_pos){.x = xmin, .y = ymax};
    generate_normals_for_polygon(result);
  }
  else {
    //dummy
    fprintf(stderr, "handed in a not 4 sided thing to find_bound_box\n");
  }
  poly->rotation = orig_rot;

}

//for finding cells that are spanned by collider
void entries_for_collider(spatial_hash_map * map, collider* collider, vector* result) {
  //think this is done, improvements are some minor adjustemnts to cut back on the number of vector_scale operations I do
  //also because _ don't have rotation or center as pointers I have to manually set/update boundboxes values a bit
  int cellW = map->cell_dim.width;
  int cellH = map->cell_dim.height;
  int collW;
  int collH;

  polygon* boundBox = collider->bbox;
  double orig_rot = boundBox->rotation;
  boundBox->rotation += collider->shape->rotation;
  boundBox->center= collider->shape->center;
  
  vector_2 bb_axis_x, bb_axis_y, vec_x, vec_y, vec_pos;
  double bbx_pos, bbx_end, bby_pos, bby_end,  proj_y_dist, proj_x_dist, mag_x, mag_y;
  virt_pos c1, c2, c3, bb_origin, pos, prev_pos, y_pos, prev_side_1, prev_side_2;
  matrix_index prev_index, temp_ind, a;
  matrix_index cells[4];
  int cell_index = 0;
  int x_count = 0, y_count;

  result->cur_size = 0;
  if (boundBox->sides == 4) {
    //start, translate points, figure out axis of bounding box
    //make unit vectors out of them, scale by the cell dimensions
    get_actual_point(boundBox, 0, &c1);
    get_actual_point(boundBox,1, &c2);
    get_actual_point(boundBox, 2, &c3);

    collW = distance_between_points(&c1, &c2);
    collH = distance_between_points(&c2, &c3);
    
    vector_between_points(&c1, &c2, &bb_axis_x);
    vector_between_points(&c2, &c3, &bb_axis_y);
    
    make_unit_vector(&bb_axis_x, &bb_axis_x);
    make_unit_vector(&bb_axis_y, &bb_axis_y);


    #ifdef SMARTY
    //code for supposedly setting vectors so that neither components are greater than a cell dimension
    virt_pos bb_axis_x_disp;
    virt_pos bb_axis_y_disp;

    vector_2_to_virt_pos(&bb_axis_x, &bb_axis_x_disp);
    vector_2_to_virt_pos(&bb_axis_y, &bb_axis_y_disp);
    
    vector_2 axis_x = (vector_2){.v1 = 1, .v2 = 0};
    vector_2 axis_y = (vector_2){.v1 = 0, .v2 = 0};
    
    
    proj_x_dist = get_projected_length(&bb_axis_x_disp, &axis_x) / cellW;
    proj_y_dist = get_projected_length(&bb_axis_x_disp, &axis_y) / cellH;
    float max = fmax(fabs(proj_x_dist), fabs(proj_y_dist));
    if (max > 1) {
      max = 1 - (max - 1);
    }
    vector_2_scale(&bb_axis_x, 1/max, &bb_axis_x);
    mag_x = vector_2_magnitude(&bb_axis_x);

    vector_2_to_virt_pos(&bb_axis_x, &bb_axis_x_disp);
    vector_2_to_virt_pos(&bb_axis_y, &bb_axis_y_disp);
    
    proj_x_dist = get_projected_length(&bb_axis_y_disp, &axis_x) / cellW;
    proj_y_dist = get_projected_length(&bb_axis_y_disp, &axis_y) / cellH;
    max = fmax(fabs(proj_x_dist), fabs(proj_y_dist));
    if (max > 1) {
      max = 1 - (max - 1);
    }
    vector_2_scale(&bb_axis_y, 1/max, &bb_axis_y);
    mag_y = vector_2_magnitude(&bb_axis_y);

    proj_x_dist = get_projected_length(&bb_axis_x_disp, &axis_x) / cellW;
    proj_y_dist = get_projected_length(&bb_axis_x_disp, &axis_y) / cellH;
    max = fmax(fabs(proj_x_dist), fabs(proj_y_dist));
    

    proj_x_dist = get_projected_length(&bb_axis_y_disp, &axis_x) / cellW;
    proj_y_dist = get_projected_length(&bb_axis_y_disp, &axis_y) / cellH;
    max = fmax(fabs(proj_x_dist), fabs(proj_y_dist));
    #else
    //code for turning entire traversal along a square grid w/ smallest dim among cell and collider
    mag_x = fmin(cellW, cellH);
    mag_x = fmin(mag_x, collW);
    mag_x = fmin(mag_x, collH);
    mag_y = mag_x;
    vector_2_scale(&bb_axis_x, mag_x, &bb_axis_x);
    vector_2_scale(&bb_axis_y, mag_y, &bb_axis_y);
    #endif
    //    scaleX = fmin(scaleX, collW);


    /*proj_x_dist = get_projected_length(&pointX, &bb_axis_y);
    proj_y_dist = get_projected_length(&pointY, &bb_axis_y);
    scaleY = fmax(proj_x_dist, proj_y_dist);
    scaleY = fmin(scaleY, collH);
    
    vector_2_scale(&bb_axis_x, scaleX, &bb_axis_x);
    vector_2_scale(&bb_axis_y, scaleY, &bb_axis_y);
    */

    bb_origin = c1;
    bby_pos = 0;
    y_count = 0;
    bby_end = (double)collH / mag_y;
    bbx_end = (double)collW / mag_x;
    
    while(bby_pos <= bby_end) {
      bbx_pos = 0;
      vector_2_scale(&bb_axis_y, bby_pos, &vec_y);
      vector_2_to_virt_pos(&vec_y, &y_pos);
      virt_pos_add(&y_pos, &bb_origin, &y_pos);

      if (y_count != 0) {
	//deals with skipped nodes across the y vector
	matrix_index curr_ind, prev_ind;
	shm_hash(map, &y_pos, &curr_ind);
	shm_hash(map, &prev_side_1, &prev_ind);
	if (!adjacent_indexes(&curr_ind, &prev_ind)) {
	 if (find_skipped_cell(&prev_side_1, &y_pos, map, &temp_ind) == 1) {
	    cells[cell_index++] = temp_ind; 
	 }
	}
      }
      
      while(bbx_pos <= bbx_end) {
	//optimize here, instead of doing scaling, could just add bbx to vec_x
	vector_2_scale(&bb_axis_x, bbx_pos, &vec_x);
	vector_2_add(&vec_x, &vec_y, &vec_pos);
	//translate vec_pos to correct corner of bb
	vector_2_to_virt_pos(&vec_pos, &pos);
	virt_pos_add(&pos, &bb_origin, &pos);
	
	shm_hash(map, &pos, &a);
	cells[cell_index++] = a;
	if (x_count != 0 && (!adjacent_indexes(&a,  &prev_index))) {
	  if (find_skipped_cell(&prev_pos, &pos, map, &temp_ind) == 1) {
	    cells[cell_index++] = temp_ind;
	  }
	}
	if (bbx_pos == bbx_end) {
	  if (y_count != 0) {
	    matrix_index curr_ind, prev_ind;
	    shm_hash(map, &pos, &curr_ind);
	    shm_hash(map, &prev_side_2, &prev_ind);
	    if (!adjacent_indexes(&curr_ind, &prev_ind)) {
	      if (find_skipped_cell(&prev_side_2, &pos, map, &temp_ind) == 1) {
		cells[cell_index++] = temp_ind; 
	      }
	    }
	  }
	  prev_side_2 = pos;
	}
	//issue of indexes in cells are already within result, call unique insert
	for(int i = 0; i < cell_index; i++) {
	  if (!already_in_vector(result, &cells[i], &matrix_index_comp)) {
	    //actuall wont work
	    //will grow space for more pointers to be stored in vector
	    //but not actually allocating space for additional matrix index structures
	    grfom(result);
	    *(matrix_index*)(result->elements[result->cur_size]) = cells[i];
	    result->cur_size++;
	    //check for size constraints, otherwise ehh,
	  }
	}
	//after all thats done
	x_count++;
	cell_index = 0;
	prev_index = a;
	prev_pos = pos;
	if (bbx_pos + 1 < bbx_end) {
	  bbx_pos += 1;
	}
	else if (bbx_pos < bbx_end) {
	  bbx_pos = bbx_end;
	}
	else {
	  bbx_pos++;
	}
      }
      x_count = 0;
      y_count++;
      prev_side_1 = y_pos;
      if (bby_pos + 1 < bby_end) {
	bby_pos++;
      }
      else if (bby_pos < bby_end) {
	bby_pos = bby_end;
      }
      else {
	bby_pos++;
      }
    }
    //again, can't use null. use vectors
    //result[count] = NULL;
  }
  else {
    //errorr
  }
  boundBox->rotation = orig_rot;
}

int find_skipped_cell (virt_pos* point1, virt_pos* point2, spatial_hash_map* map,  matrix_index* result) {
  int MIDPOINT_LOOPS = 5, ret = 0;
  virt_pos a = *point1, b = *point2, midp;
  matrix_index a_ind, b_ind, midp_ind;
  shm_hash(map, &a, &a_ind);
  shm_hash(map, &b, &b_ind);
  if (matrix_index_difference(&a_ind,&b_ind) == 2) {
    for (int i = 1; i < MIDPOINT_LOOPS; i++) {
      //for getting midpoint, need previouse vector/point thing
      virt_pos_midpoint(&a, &b, &midp);
      shm_hash(map, &midp, &midp_ind);
      if (matrix_index_compare(&a_ind,&midp_ind) == 0) {
	a = midp;
      }
      else if (matrix_index_compare(&b_ind,&midp_ind) == 0) {
	b = midp;
      }
      else {
	//found missing cell, add?
	*result = midp_ind;
	ret = 1;
	break;
      }
    }
  }
  //else indexes are either adjacent/same, or there is more than 1 skipped cell
  return ret;
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


//nah, still too much work
//going to try solution of adding a data field to collider node, just an int
//in here, check if == 0. if true, set to 1, add to colliders
//done. no need for multiple functions and for loops which overallocate and have to guess/reason how many unique nodes there has to be

//well, still think i need to split it up into 2 functions for avoiding mallocs
//one, goes through cells, checks for 0, changes to 1, returns number of unique colliders

//using return value, create array of colliders, hand in to second funciton
//same traversal, checks for 1',s adds to result, changes back to zero

//probably infeasable, but when making compound collider objects, could change field to a pointer, so that collider list nodes of same overall object but different colliders all have a pointer to a shared int.

int DEFAULT = 0;
int VISITED = 1;
int COUNTED = 2;

int number_of_unique_colliders_in_entries(spatial_hash_map* map, vector* entries) {
  int count = 0;
  spatial_map_cell* cell;
  gen_node* curr ;
  collider_list_node* cln;
  //wondering how to do this, since I need to remember which colliders I find
  //thinking of allocating a linked list onto stack, and doing it that way
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

collider_list_node* make_cln_from_collider(collider* coll) {
  collider_list_node* new = malloc(sizeof(collider_list_node));
  new->collider = coll;
  new->status = DEFAULT;
  return new;
}

//new idea, hand in a  collider list node, use pointer comparison on collider(maybe something else eventually once I start making objects w/ multiple colliders) in node, only add

/*
int number_of_stupid_cellss(spatial_hash_map* map, vector* entries) {
  int count = 0;
  int sum = 0;
  spatial_map_cell* cell;
  //wondering how to do this, since I need to remember which colliders I find
  //thinking of allocating a linked list onto stack, and doing it that way
  for(int i = 0; i < entries->cur_size; i++) {
    cell = get_entry_in_shm(map, elementAt(entries,i));
    sum += number_of_colliders_in_cell(cell);
    count++;
  }
  collider* colliders[sum - count + 2];
  count = 0;
  for(int i = 0; i < entries->cur_size; i++) {
    cell = get_entry_in_shm(map, elementAt(entries,i));
    //travers list in cell, grab collider list nodes, do unique insert on array
    gen_node* curr = cell->colliders_in_cell->start;
    collider* coll;
    while(curr != NULL) {
      coll = ((collider_list_node*)(curr->stored))->collider;
      if (collider_unique_insert_array(coll, colliders) == 1) {
	count++;
      }
    }
  }
  return count;
}

int number_of_colliders_in_cell(spatial_map_cell* cell) {
  //this won't work if there are double entries for colliders in cell
  //which would fuck up a lot of other things as well
  //could print out result and use as a basic test against double adding
  int count = 0;
  gen_node* curr = cell->colliders_in_cell->start;
  while(curr != NULL) {
    count++;
    curr = curr->next;
  }
  return count;
}
*/
//either  a smcell or mylist, idk....
spatial_map_cell* get_entry_in_shm(spatial_hash_map* map, matrix_index* index) {
  void* data = getDataAtIndex(map->hash_map, index->x_index, index->y_index);
  return (spatial_map_cell*)data;
}
void shm_hash(spatial_hash_map* map, virt_pos* pos, matrix_index* result) {
  draw_virt_pos(getCam(), pos);
  int x_index = pos->x / map->cell_dim.width;
  int y_index = pos->y / map->cell_dim.height;
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



//all of the above was for the rough early-exclusion collision detection
//for for fine grained stuff. Planning on using Seperation along axis

//first, need to redesign game shapes to be arbitrary polygons


//assuming I have some array of collider list nodes of potentially intersecting things
//results can be colliders in multi that collide with mono. 
int check_for_collisions(collider* mono, collider** multi, collider** results) {
  //unsure what to do here
  int i = 0, count = 0;
  collider* check = multi[i];
  while(check != NULL) {
    if (mono != check) {
      if (do_polygons_intersect(mono->shape, check->shape)) {
	results[count] = check;
	count++;
      }
    }
  }
  results[count] = NULL;
  return count;
}


matrix_index* createMatrixIndex() {
  matrix_index* new = malloc(sizeof(new));
  new->x_index = 0;
  new->y_index = 0;
  return new;
}


//soritng library stuff maybe
/*
int matrix_index_compare_wrapper(void* m, void* b) {
  return matrix_index_compare((matrix_index*)m,(matrix_index*)b);
}


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
      result = 1;
    }
    else if (m1->x_index < m2->x_index) {
      result = 1;
    }
    else {
      result = 0;
    }
  }
  return result;
}

comparer matrix_comp = {.compare= matrix_index_compare_wrapper};
int matrix_index_unique_insert(gen_list* list, gen_node* data) {
  return unique_insert(list, data, &matrix_index_comp);
}

int unique_insert(gen_list* list, gen_node* data, comparer* comp) {
  //assumes list has been sorted
  //returns 1 on success, -1 if duplicate data in list
  int insert = 0;
  int prev_compare = 0, compare = 0;;
  gen_node* curr = list->start;
  if (curr == NULL) {
    prependToGen_list(list, data);
    insert = 1;
  }
  else {
    compare = comp->compare(data->stored, curr->stored);
    if ( compare < 0) {
      prependToGen_list(list, data);
      insert = 1;
    }
    else if (compare == 0) {
      insert = -1;
    }
    prev_compare = compare;
    curr = curr->next;
    while(!insert) {
      if (curr == NULL) {
	//end of list
	appendToGen_list(list, data);
	insert = 1;
      }
      else {
	compare = comp->compare(data->stored, curr->stored);
	if (prev_compare > 0 && compare < 0) {
	  curr->prev->next = data;
	  data->prev = curr->prev;
	  curr->prev = data;
	  data->next = curr;
	  insert = 1;
	}
	else if (compare == 0) {
	  insert = -1;
	}
	prev_compare = compare;
      }
    }
  }
}

int matrix_index_unique_insert_array(matrix_index* insert, matrix_index* array) {
  int done = 0;
  int size = 0;
  int ret;
  while(!done) {
    //this won't work, why I wanted to use vectores
    if (array[size] == NULL) {
      done = 1;
    }
    else {
      size++;
    }
  }
  matrix_index* array2[size];
  for (int i = 0; i <= size; i++) {
    array2[i] = &(array[i]);
  }
  ret = unique_insert_array(insert, array2, size, &matrix_comp);
  if (ret == 1) {
    for (int i = 0; i <= size; i++) {
      array[i] = *(array2[i]);
    }
    array[size + 1] = NULL;
  }
  
}

int unique_insert_array(void* insert, void** array, int fei, comparer* comp) {
  //assumes array given is already sorted
  //also assumes that array is an array of pointers

  //idea, do a binary search through array, until start and end are adjacent indexes with start < insert and insert < end
  int start, end, search, inserted, insert_index;
  void* item_at_search;
  start = 0;
  end = fei;
  inserted = 0;
  while(!inserted) {
    //need some case for if insert is already in array, in which case don't do anything
    search = (start + end) / 2;
    item_at_search = array[search];
    if ((end - start) <= 1) {
      insert_index = end;
      inserted = 1;
    }
    else if (comp->compare(item_at_search, insert) < 0 ) {
      start = search;
    }
    else if (comp->compare(item_at_search, insert) > 0) {
	end = search;
    }
    else {
      //already in array,
      inserted = -1;
    }
  }
  if (inserted == 1) {
    for (int i = fei; i > insert_index; i--) {
      array[i] = array[i - 1];
    }
    array[inserted] = insert;
  }
}
 */

