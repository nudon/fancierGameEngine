#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "geometry.h"

vector_2* zero_vec = &((vector_2){.v1 = 0, .v2 = 0});

virt_pos* zero_pos = &((virt_pos){.x = 0, .y = 0});

static void init_point(virt_pos* p) {
  p->x = 0;
  p->y = 0;
}

static void init_vector(vector_2* v) {
  v->v1 = 0;
  v->v2 = 0;
}

polygon* createPolygon(int sides) {
  polygon* new = malloc(sizeof(polygon));
  new->sides = sides;
  new->scale = 1;
  new->rotation = 0;
  new->center = (virt_pos){.x = 0, .y = 0};
  init_point(&(new->center));
  new->corners = malloc(sizeof(virt_pos) * sides);
  new->normals = malloc(sizeof(vector_2) * sides);
  for (int i = 0; i < sides; i++) {
    init_point(&(new->corners[i]));
    init_vector(&(new->normals[i]));
  }
  return new;
}

polygon* createNormalPolygon(int sides) {
  polygon* poly = createPolygon(sides);
  make_normal_polygon(poly);
  generate_normals_for_polygon(poly);
  return poly;
}

void generate_normals_for_polygon(polygon* poly) {
  //this needs a bit more work
  //just blindly rotating by 90 degrees could point normals inwards
  //have to make normal point away from center
  //can do this by comparing point on side, point displaced by normal vector, and center of polygon
  int sides = poly->sides;
  virt_pos p1, p2, cent, midp, disp;
  double  cent_l, midp_l, disp_l;
  vector_2 temp;
  cent = poly->center;
  //corner points are stored relative to center
  //just making center zero should work
  cent = *zero_pos;
  for (int i = 0; i < sides; i++) {
    p1 = poly->corners[i];
    p2 = poly->corners[(i + 1) % sides];
    vector_between_points(&p1, &p2, &temp);
    vector_2_rotate(&temp, M_PI / 2, &temp);

    virt_pos_midpoint(&p1, &p2, &midp);
    vector_2_to_virt_pos(&temp, &disp);
    virt_pos_add(&midp, &disp, &disp);


    cent_l = get_projected_length(&cent, &temp);
    midp_l = get_projected_length(&midp, &temp);
    disp_l = get_projected_length(&disp, &temp);

    if ( (cent_l <= midp_l && midp_l <= disp_l) ||
	 (disp_l <= midp_l && midp_l <= cent_l)) {

    }
    else {
      vector_2_scale(&temp, -1, &temp);
    }
    poly->normals[i] = temp;
    
    
  }
}

//had idea for a general normal polygon generator
//just have center and some radial length
//create first point, then rotate 360/sides, for second point, and so on.
void make_normal_polygon(polygon* poly) {
  int sides = poly->sides;
  double rot_delta = M_PI * 2 / sides;
  virt_pos first = (virt_pos){.x = 10, .y = 0};
  if (sides > 2) {
    poly->corners[0] = first;
    for (int i = 1; i < sides; i++) {
      virt_pos_rotate(&first, rot_delta * i, &(poly->corners[i]));
    }
    if (sides % 2 == 0) {
      poly->rotation = rot_delta / 2;
    }
  }
  else {
    printf("you are dumb, have fun finding this error");
  }
}

void make_square(polygon* poly) {
  if (poly->sides == 4) {
    poly->corners[0] = (virt_pos){.x = -1, .y = -1};
    poly->corners[1] = (virt_pos){.x = 1, .y = -1};
    poly->corners[2] = (virt_pos){.x = 1, .y = 1};
    poly->corners[3] = (virt_pos){.x = -1, .y = 1};
  }
}

void stretch_deform_vert(polygon* poly, double amount) {
  int sides = poly->sides;
  for (int i = 0; i < sides; i++) {
    poly->corners[i].y *= amount;
  }
}

void stretch_deform_horz(polygon* poly, double amount) {
  int sides = poly->sides;
  for (int i = 0; i < sides; i++) {
    poly->corners[i].x *= amount;
  }
}

//could do stretch deform for arbitrary lines. just need to project each point onto line, get orth and parrallell components, scale the parllell components and add to orthogonal.



void freePolygon(polygon* poly) {
  free(poly->corners);
  free(poly->normals);
  free(poly);
}

void get_actual_point(polygon* poly, int i, virt_pos* result) {
  result->x = poly->corners[i].x * poly->scale;
  result->y = poly->corners[i].y * poly->scale;
  virt_pos_rotate(result, poly->rotation, result);
  //scale result by poly's scale

  virt_pos_add(result, &(poly->center), result);
}

void get_actual_normal(polygon* poly, int i, vector_2* result) {
  *result = poly->normals[i];
  vector_2_rotate(result, poly->rotation, result);
}


 int do_polygons_intersect(polygon* p1, polygon* p2) {
   //returns 1 if true, zero if false
  //was going to consider doing this so that the origin is the center of p1.
  //worried about odd things happenening when both polygons are far away, due to shenanigans of dealing with angle between objects and float/double precision
  //easy for p1, since it's corners should already be relative to center
  //p1, corners will be offset by difference between p2 and p1's absolute centers.
  //intending for normals to just point in same direction as surface, no need to change that with new origin
  
  //need to do all below for potentially every normal of both polygons
  double offset_x, offset_y;
  double p1_min, p1_max, p2_min, p2_max;
  virt_pos relative_point;
  vector_2 normal_vector;
  int isDone = 0, ret = 1, index = 0, polygon = 1;;
  fprintf(stderr, "in dpi\n");
  while(!isDone) {
    if (polygon == 1) {
      if (index < p1->sides) {
	get_actual_normal(p1, index, &normal_vector);
	index++;
      }
      else {
	polygon = 2;
	index = 0;
      }
    }
    if (polygon == 2) {
      if (index < p2->sides) {
	get_actual_normal(p2, index, &normal_vector);
	index++;
	if (index >= p2->sides) {
	  isDone = 1;
	}
      }
    }
    //if still worried about double range doing odd things
    //could pass in difference between p1 and p2 centers, should workn
    
    extreme_projections_of_polygon(p1, &p1->center, &normal_vector, &p1_min, &p1_max);
    extreme_projections_of_polygon(p2, &p1->center, &normal_vector, &p2_min, &p2_max);
     if ( p1_max < p2_min || p2_max < p1_min) {
      isDone = 1;
      ret = 0;
    }
    
  }
  return ret;  
}

int find_mtv_of_polygons(polygon* p1, polygon* p2, vector_2* mtv) {
  //returns 1 if true, zero if false
  //basically same thing as do_polygons_intersect
  //though this will find all intersecting sides and supply an mtv

  double offset_x, offset_y;
  double p1_min, p1_max, p2_min, p2_max, max, min;
  virt_pos relative_point;
  vector_2 normal_vector;
  int isDone = 0, ret = 0, index = 0, polygon = 1;
  vector_2 mtv_loc = *zero_vec;
  double mtv_mag = -12;
  double temp_mag;
  fprintf(stderr, "in mtv\n");
  while(!isDone) {
    if (polygon == 1) {
      if (index < p1->sides) {
	get_actual_normal(p1, index, &normal_vector);
	index++;
      }
      else {
	polygon = 2;
	index = 0;
      }
    }
    if (polygon == 2) {
      if (index < p2->sides) {
	get_actual_normal(p2, index, &normal_vector);
	index++;
	if (index >= p2->sides) {
	  isDone = 1;
	}
      }
    }
    //if still worried about double range doing odd things
    //could pass in difference between p1 and p2 centers, should workn
    extreme_projections_of_polygon(p1, &p1->center, &normal_vector, &p1_min, &p1_max);
    extreme_projections_of_polygon(p2, &p1->center, &normal_vector, &p2_min, &p2_max);
    //fprintf(stderr, "normal is %d, %d\n", normal_vector.v1, normal_vector.v2);
     if ( p1_max > p2_min && p2_max > p1_min) {
       temp_mag = fabs((fmin(p1_max, p2_max)) - (fmax(p1_min, p2_min)));
       if (temp_mag < mtv_mag || mtv_mag < 0) {
	 mtv_mag = temp_mag;
	 mtv_loc = normal_vector;
       }
       //isDone = 1;
       ret = 1;
    }
    
  }
  *mtv = mtv_loc;
  return ret;  
}

//including an optional neworigin as a vague stress releif options
//having fears that floating point arithmatic getting imprecice at higher values could mess up things
void extreme_projections_of_polygon(polygon* check,virt_pos* new_origin,vector_2* line, double* min_result, double* max_result) {
  double min, max, temp, rotation;
  virt_pos relative_point, origin, offset;
  if (new_origin == NULL) {
    origin.x = 0;
    origin.y = 0;
  }
  else {
    origin = *new_origin;
  }
  offset.x = -origin.x;
  offset.y = -origin.y;
  for (int i = 0; i < check->sides; i++) {
    get_actual_point(check,i, &relative_point);
    //relative_point.x = check->corners[i].x;
    //relative_point.y = check->corners[i].y;
    relative_point.x += offset.x;
    relative_point.y += offset.y;
    
    temp = get_projected_length(&relative_point, line);
    if (i == 0 ) {
      max = temp;
      min = temp;
    }
    else {
      if (temp > max) {
	max = temp;
      }
      else if (temp < min) {
	min = temp;
      }
    }
  }
  *min_result = min;
  *max_result = max;
}

int is_virt_pos_in_dir_of_normal(vector_2* norm, virt_pos* det, virt_pos* point) {
  virt_pos disp;
  double  det_l, disp_l, point_l;
  int ret = 0;
  vector_2_to_virt_pos(norm, &disp);
  virt_pos_add(&disp, det, &disp);
  
  det_l = get_projected_length(det, norm);
  disp_l = get_projected_length(&disp, norm);
  point_l = get_projected_length(point, norm);

  if ((det_l < disp_l && det_l < point_l) ||
      (det_l > disp_l && det_l > point_l)) {
    ret = 1;
  }
  return ret;
}

double get_projected_length_vec(vector_2* vec, vector_2* line) {
  //float delta = atan2(line->v2, line->v1) - atan2(point->y, point->x);
  double projectedLength = 0;
  if (!isZeroVec(line)) {
    float delta = atan2(vec->v2, vec->v1) - atan2(line->v2, line->v1);
    //possiblyh delta is over 180 or pi, kind of dumb
    if (delta > M_PI) {
      delta = delta - M_PI;
    }
    double hyp = vector_2_magnitude(vec);
    projectedLength = cos(delta) * hyp;
  }
  return projectedLength;
}

void get_parallell_comp(vector_2* vec, vector_2* line, vector_2* result) {
  //project_point_onto_line(point, line, result); , no, bad points
  double mag = get_projected_length_vec(vec, line);
  vector_2 unit;
  make_unit_vector(line, &unit);
  vector_2_scale(&unit, mag, result);
  
}

void get_orthogonal_comp(vector_2* vec, vector_2* line, vector_2* result) {
  //could either rotate line and project onto that, but then component might be negative if I'm done
  //or, get parallell component, then subtract out of point
  /*
  virt_pos loc;
  project_point_onto_line(point, line, &loc);
  vector_2_subtract(point, &loc, result);

  */
  vector_2 parallell;
  get_parallell_comp(vec, line, &parallell);
  vector_2_sub(vec, &parallell, result);
}

void decompose_vector(vector_2* vec, vector_2* line, vector_2* p, vector_2* o) {
  get_parallell_comp(vec, line, p);
  vector_2_sub(vec, p, o);
}

void  project_point_onto_line(virt_pos* point, vector_2* line, virt_pos* result) {
  //imagined for 2d, 3d need not apply
  //also, technichally for sat all I  need is the projectedLenght
  double projectedLength = get_projected_length(point, line);
  vector_2 unit;
  make_unit_vector(line, &unit);
  result->x = unit.v1 * projectedLength;
  result->y = unit.v2 * projectedLength;
}

int isZeroPos(virt_pos* pos) {
  return (pos->x == 0 && pos->y ==0);
}

int isZeroVec(vector_2* vec) {
  return (vec->v1 == 0 && vec->v2 == 0);
}

int isCloseEnoughToZeroVec(vector_2* vec) {
  double error = .0001;
  double mag = vector_2_magnitude(vec);
  int ret = 0;
  if ( mag < error) {
    ret = 1;
  }
  return ret;
}

double get_projected_length(virt_pos* point, vector_2* line) {
  //float delta = atan2(line->v2, line->v1) - atan2(point->y, point->x);
  double projectedLength = 0, temp;
  
  if (!isZeroVec(line)) {
    float delta = atan2(point->y, point->x) - atan2(line->v2, line->v1);
    //possiblyh delta is over 180 or pi, kind of dumb
    /*
    if (delta > M_PI) {
      temp = delta - M_PI;
      delta = M_PI - delta;
    }
    */
    double hyp = distance_from_origin(point);
    projectedLength = cos(delta) * hyp;
  }
  return projectedLength;
}



double distance_from_origin(virt_pos* point) {
  return sqrt(point->x * point->x + point->y * point->y);
}

double distance_between_points(virt_pos* p1, virt_pos* p2) {
  double t1, t2;
  t1 = p1->x - p2->x;
  t2 = p1->y - p2->y;
  return sqrt(t1 * t1 + t2 * t2);
}

 int is_a_unit_vector(vector_2* vec) {
   int ret;
   double error = .001;
   double dist = vector_2_magnitude(vec);
   if (dist < 1 + error && dist > 1 - error) {
     ret = 1;
   }
   else {
     ret = 0;
   }
   return ret;
 }

void make_unit_vector(vector_2* op, vector_2* result) {
  double error = .001;
  
  double mag;
  if ( !is_a_unit_vector(op)) {
    mag = vector_2_magnitude(op);
    result->v1 = op->v1 / mag;
    result->v2 = op->v2 / mag;
  }
  else {
    *result = *op;
  }
  assert(is_a_unit_vector(result) == 1);
}


void vector_2_add(vector_2* x, vector_2* y, vector_2* result) {
  result->v1 = x->v1 + y->v1;
  result->v2 = x->v2 + y->v2;
}

void vector_2_sub(vector_2* x, vector_2* y, vector_2* result) {
  result->v1 = x->v1 - y->v1;
  result->v2 = x->v2 - y->v2;
}

void vector_2_mul(vector_2* x, vector_2* y, vector_2* result) {
  result->v1 = x->v1 * y->v1;
  result->v2 = x->v2 * y->v2;
}

void vector_2_div(vector_2* x, vector_2* y, vector_2* result) {
  result->v1 = x->v1 / y->v1;
  result->v2 = x->v2 / y->v2;
}

void vector_2_scale(vector_2* v, double c, vector_2* result) {
  result->v1 = v->v1 * c;
  result->v2 = v->v2 * c;
 }

 double vector_2_magnitude(vector_2* v) {
   return sqrt(v->v1 * v->v1 + v->v2 * v->v2);
 }
 
void vector_2_rotate(vector_2* vec, double radians, vector_2* result) {
  //just hardcode in rotation matrix
  // | cosT, -sinT |
  // | subT,  cotT |
  static double loc_radians = 0;
  static double sinVal = 0;
  static double cosVal = 1;
  vector_2 copy =  *vec;
  if (loc_radians != radians) {
    sinVal = sin(radians);
    cosVal = cos(radians);
    loc_radians = radians;
  }
  result->v1 = cosVal * copy.v1 - sinVal * copy.v2;
  result->v2 = sinVal * copy.v1 + cosVal * copy.v2;
}


void virt_pos_add(virt_pos* p1, virt_pos* p2, virt_pos* result) {
  result->x = p1->x + p2->x;
  result->y = p1->y + p2->y;
}

void virt_pos_sub(virt_pos* p1, virt_pos* p2, virt_pos* result) {
  result->x = p1->x - p2->x;
  result->y = p1->y - p2->y;
}


void virt_pos_midpoint(virt_pos* p1, virt_pos* p2, virt_pos* result) {
  virt_pos_add(p1, p2, result);
  result->x /= 2;
  result->y /= 2;
}


void virt_pos_rotate(virt_pos* pos, double radians, virt_pos* result) {
  vector_2 pos_vec = {0,0}, result_vec = {0,0};
  virt_pos_to_vector_2(pos, &pos_vec);
  vector_2_rotate(&pos_vec, radians, &result_vec);
  vector_2_to_virt_pos(&result_vec, result);
}
 
void virt_pos_to_vector_2(virt_pos* in, vector_2* out) {
  out->v1 = in->x;
  out->v2 = in->y;
}

void vector_2_to_virt_pos(vector_2* in, virt_pos* out) {
  out->x = round(in->v1);
  out->y = round(in->v2);
}

void vector_between_points( virt_pos* p1, virt_pos* p2, vector_2* result) {
  //p1 is start, p2 is end
  result->v1 = p2->x - p1->x;
  result->v2 = p2->y - p1->y;
}

void print_vector(vector_2* vec) {
  fprintf(stderr, "vector is %f, %f\n", vec->v1, vec->v2);
}
