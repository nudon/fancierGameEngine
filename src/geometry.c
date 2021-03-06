#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "geometry.h"

struct polygon_struct {
  int sides;
  double scale;
  double rotation;
  virt_pos* center;
  //base corners, which contain relative shape of object but no rotation/scale
  virt_pos* base_corners;
  //rotated corners, to store rotated posistions of corners to cut back calculations
  virt_pos* corners;
  //base normals and rotated corners
  //normal[i] is normal vector of side p[i]-p[i+1] 
  vector_2* base_normals; 
  vector_2* normals;
  virt_pos rotation_offset;
  int x_reflection;
  int y_reflection;
};

vector_2* zero_vec = &((vector_2){.v1 = 0, .v2 = 0});

virt_pos* zero_pos = &((virt_pos){.x = 0, .y = 0});

vector_2* x_axis = &((vector_2){.v1 = 1, .v2 = 0});

vector_2* y_axis = &((vector_2){.v1 = 0, .v2 = 1});

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
  new->center = malloc(sizeof(virt_pos));
  new->corners = malloc(sizeof(virt_pos) * sides);
  new->base_corners = malloc(sizeof(virt_pos) * sides);
  new->normals = malloc(sizeof(vector_2) * sides);
  new->base_normals = malloc(sizeof(vector_2) * sides);
  new->x_reflection = 1;
  new->y_reflection = 1;
  init_point(new->center);
  for (int i = 0; i < sides; i++) {
    init_point(&(new->corners[i]));
    init_point(&(new->base_corners[i]));
    init_vector(&(new->normals[i]));
    init_vector(&(new->base_normals[i]));
  }
  new->rotation_offset = *zero_pos;
  return new;
}

polygon* clonePolygon(polygon* src) {
  polygon* new = malloc(sizeof(polygon));
  int sides = src->sides;
  new->sides = sides;
  new->scale = src->scale;
  new->rotation = src->rotation;
  new->center = malloc(sizeof(virt_pos));
  *(new->center) = *(src->center);
  new->corners = malloc(sizeof(virt_pos) * sides);
  new->base_corners = malloc(sizeof(virt_pos) * sides);
  new->normals = malloc(sizeof(vector_2) * sides);
  new->base_normals = malloc(sizeof(vector_2) * sides);
  new->x_reflection = src->x_reflection;
  new->y_reflection = src->y_reflection;
  init_point(new->center);
  for (int i = 0; i < sides; i++) {
    new->corners[i] = src->corners[i];
    new->base_corners[i] = src->base_corners[i];
    new->normals[i] = src->normals[i];
    new->base_normals[i] = src->base_normals[i];
  }
  new->rotation_offset = *zero_pos;
  return new;
}

polygon* createNormalPolygon(int sides) {
  polygon* poly = createPolygon(sides);
  make_normal_polygon(poly);
  generate_normals_for_polygon(poly);
  recalc_corners_and_norms(poly);
  return poly;
}

polygon* createRectangle(int width, int height) {
  polygon* r = createPolygon(4);
  make_square(r);
  stretch_deform_vert(r, fmax(1, height / 2));
  stretch_deform_horz(r, fmax(1, width / 2));
  generate_normals_for_polygon(r);
  return r;
}


void generate_normals_for_polygon(polygon* poly) {
  int sides = poly->sides;
  virt_pos p1, p2, cent, midp, disp;
  double  cent_l, midp_l, disp_l;
  vector_2 temp;
  cent = *zero_pos;
  for (int i = 0; i < sides; i++) {
    get_base_point(poly, i, &p1);
    get_base_point(poly, (i + 1) % sides , &p2);
    temp = vector_between_points(&p1, &p2);
    vector_2_rotate(&temp, M_PI / 2, &temp);

    virt_pos_midpoint(&p1, &p2, &midp);
    vector_2_to_virt_pos(&temp, &disp);
    virt_pos_add(&midp, &disp, &disp);


    cent_l = get_projected_length(&cent, &temp);
    midp_l = get_projected_length(&midp, &temp);
    disp_l = get_projected_length(&disp, &temp);

    if (!((cent_l <= midp_l && midp_l <= disp_l) ||
	  (disp_l <= midp_l && midp_l <= cent_l))) {
      vector_2_scale(&temp, -1, &temp);
    }
    make_unit_vector(&temp, &temp);
    poly->base_normals[i] = temp;
  }
  recalc_corners_and_norms(poly);
}

void make_normal_polygon(polygon* poly) {
  int sides = poly->sides;
  double rot_delta = M_PI * 2 / sides;
  virt_pos first = (virt_pos){.x = 10, .y = 0};
  virt_pos temp = *zero_pos;
  if (sides > 2) {
    set_base_point(poly, 0, &first);
    for (int i = 1; i < sides; i++) {
      virt_pos_rotate(&first, rot_delta * i, &temp);
      set_base_point(poly, i, &temp);
    }
    if (sides % 2 == 0) {
      set_rotation(poly, rot_delta / 2);
    }
  }
  else {
    printf("you can't make a polygon with < 3 sides");
  }
}

void make_square(polygon* poly) {
  if (poly->sides == 4) {
    set_base_point(poly, 0, &(virt_pos){.x = -1, .y = -1});
    set_base_point(poly, 1, &(virt_pos){.x = 1, .y = -1});
    set_base_point(poly, 2, &(virt_pos){.x = 1, .y = 1});
    set_base_point(poly, 3, &(virt_pos){.x = -1, .y = 1});
  }
}

void stretch_deform_vert(polygon* poly, double amount) {
  int sides = poly->sides;
  virt_pos temp = *zero_pos;
  for (int i = 0; i < sides; i++) {
    get_base_point(poly, i, &temp);
    temp.y *= amount;
    set_base_point(poly, i, &temp);
  }
}

void stretch_deform_horz(polygon* poly, double amount) {
  int sides = poly->sides;
  virt_pos temp = *zero_pos;
  for (int i = 0; i < sides; i++) {
    get_base_point(poly, i, &temp);
    temp.x *= amount;
    set_base_point(poly, i, &temp);
  }
}

void set_scale(polygon* p, double scale) {
  p->scale = scale;
  recalc_corners_and_norms(p);
}

double get_scale(polygon* p) {
  return p->scale;
}

void freePolygon(polygon* poly) {
  free(poly->base_corners);
  free(poly->corners);
  free(poly->base_normals);
  free(poly->normals);
  free(poly);
}

int get_sides(polygon* p) {
  return p->sides;
}

void get_actual_point(polygon* poly, int i, virt_pos* result) {
  *result = poly->corners[i];
}

void get_base_point(polygon* poly, int i, virt_pos* result) {
  result->x = poly->base_corners[i].x;
  result->y = poly->base_corners[i].y;
}

void set_reflections(polygon* p, int x_r, int y_r) {
  if (x_r * x_r != 1 || y_r * y_r != 1) {
    fprintf(stderr, "error, cant set x and y reflections to %d and %d, vals may only be -1 or 1\n", x_r, y_r);
  }
  else {
    if (p->x_reflection != x_r || p->y_reflection != y_r) {
      p->x_reflection = x_r;
      p->y_reflection = y_r;
      recalc_corners_and_norms(p);
    }
  }
}

int get_x_reflection(polygon* p) {
  return p->x_reflection;
}

int get_y_reflection(polygon* p) {
  return p->y_reflection;
}

void set_base_point(polygon* poly, int i, virt_pos* set) {
  poly->base_corners[i].x = set->x;
  poly->base_corners[i].y = set->y;
}

void get_actual_normal(polygon* poly, int i, vector_2* result) {
  *result = poly->normals[i];
}

virt_pos get_center(polygon* poly) {
  return *poly->center;
}

void set_center(polygon* poly, virt_pos* val) {
  if (val == NULL) {
    poly->center = NULL;
  }
  else {
    poly->center->x = val->x;
    poly->center->y = val->y;
    recalc_corners(poly);
  }
}

void set_center_p(polygon* p, virt_pos* cent_p) {
  p->center = cent_p;
}

void set_rotation(polygon* poly, double new) {
  double ang = clamp_rotation(new);
  poly->rotation = ang;
  recalc_corners_and_norms(poly);
}

void recalc_corners_and_norms(polygon* poly) {
  double ang = poly->rotation;
  recalc_corners(poly);
  for (int i = 0; i < poly->sides; i++) {
    vector_2_rotate(&(poly->base_normals[i]), ang, &(poly->normals[i]));
  }
}

void recalc_corners(polygon* poly) {
  virt_pos temp = *zero_pos;
  double ang = poly->rotation;
  for (int i = 0; i < poly->sides; i++) {
    temp = poly->base_corners[i];
    temp.x = temp.x * poly->scale;
    temp.y = temp.y * poly->scale;
    temp.x *= poly->x_reflection;
    temp.y *= poly->y_reflection;
    
    virt_pos_rotate(&temp, ang, &temp);

    virt_pos_add(&temp, poly->center, &temp);
    poly->corners[i] = temp;
  }  
}

double get_rotation(polygon* poly) {
  return poly->rotation;
}

void set_rotation_offset(polygon* poly, virt_pos* offset) {
  poly->rotation_offset = *offset;
  poly->rotation_offset.x *= poly->x_reflection;
  poly->rotation_offset.y *= poly->y_reflection;
}

virt_pos get_rotation_offset(polygon* p) {
  virt_pos t = p->rotation_offset;
  t.x *= p->x_reflection;
  t.y *= p->y_reflection;
  return t;
}
//returns 1 if true, zero if false
int do_polygons_intersect(polygon* p1, polygon* p2) {
  double p1_min, p1_max, p2_min, p2_max;
  vector_2 normal_vector;
  int isDone = 0, ret = 1, index = 0, polygon = 1;;
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
    extreme_projections_of_polygon(p1, p1->center, &normal_vector, &p1_min, &p1_max);
    extreme_projections_of_polygon(p2, p1->center, &normal_vector, &p2_min, &p2_max);
     if ( p1_max < p2_min || p2_max < p1_min) {
      isDone = 1;
      ret = 0;
    }
    
  }
  return ret;  
}

//returns 1 if the polygons intersect and mtv is filled with minimum translation vector to solve collisions
//returns 0 if no collision
int find_mtv_of_polygons(polygon* p1, polygon* p2, vector_2* mtv) {
  double p1_min, p1_max, p2_min, p2_max;
  vector_2 normal_vector;
  int isDone = 0, ret = 0, index = 0, polygon = 1;
  vector_2 mtv_loc = *zero_vec;
  double mtv_mag = -12;
  double temp_mag;
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
    extreme_projections_of_polygon(p1, p1->center, &normal_vector, &p1_min, &p1_max);
    extreme_projections_of_polygon(p2, p1->center, &normal_vector, &p2_min, &p2_max);
    //there is overlap of shapes along axis
     if ( p1_max > p2_min && p2_max > p1_min) {
       temp_mag = fmin(fabs(p2_max - p1_min), fabs(p1_max - p2_min));
       if (temp_mag < mtv_mag || mtv_mag < 0) {
	 mtv_mag = temp_mag;
	 mtv_loc = normal_vector;
       }
       ret = 1;
     }
     //there is a gap
     else {
       isDone = 1;
       ret = 0;
     }
       
    
  }
  vector_2_scale(&mtv_loc, mtv_mag, &mtv_loc);
  *mtv = mtv_loc;
  return ret;  
}

void get_normals_of_collision(polygon* p1, polygon* p2, vector_2* normal, vector_2* body1_norm, vector_2* body2_norm) {
  double l1, l2;
  virt_pos b1c = get_center(p1);
  virt_pos b2c = get_center(p2);
  l1 = get_projected_length_pos(&b1c, normal);
  l2 = get_projected_length_pos(&b2c, normal);
  *body1_norm = *normal;
  *body2_norm = *body1_norm;
  if (l1 < l2) {
    vector_2_scale(body1_norm, -1, body1_norm);
  }
  else {
    vector_2_scale(body2_norm, -1, body2_norm);
  }
}


int calc_contact_point(polygon* p1, polygon* p2, vector_2* mtv, virt_pos* result) {
  virt_pos p1_min, p1_max, p1_sec_min, p1_sec_max, p2_min, p2_max, p2_sec_min, p2_sec_max;
  virt_pos p1_e, p1_e_sec, p2_e, p2_e_sec;
  double d1, d2;

  double thresh = 1.5;
  virt_pos contact_point = *zero_pos;
  int ret = 0;
  
  tmi_points_of_polygon(p1, mtv, &p1_min, &p1_max, &p1_sec_min, &p1_sec_max);
  tmi_points_of_polygon(p2, mtv, &p2_min, &p2_max, &p2_sec_min, &p2_sec_max);
  
  d1 = fabs(get_projected_length(&p1_min, mtv) - get_projected_length(&p2_max, mtv));
  d2 = fabs(get_projected_length(&p2_min, mtv) - get_projected_length(&p1_max, mtv));

  if (d1 < d2) {
    p1_e = p1_min;
    p1_e_sec = p1_sec_min;
    p2_e = p2_max;
    p2_e_sec = p2_sec_max;
  }
  else {
    p1_e = p1_max;
    p1_e_sec = p1_sec_max;
    p2_e = p2_min;
    p2_e_sec = p2_sec_min;    
  }

  d1 = fabs(get_projected_length(&p1_e, mtv) - get_projected_length(&p1_e_sec, mtv));
  d2 = fabs(get_projected_length(&p2_e, mtv) - get_projected_length(&p2_e_sec, mtv));
  
  if (d1 < thresh && d2 < thresh) {
    d1 = distance_between_points(&p1_e, &p1_e_sec);
    d2 = distance_between_points(&p2_e, &p2_e_sec);
    if (d1 < d2) {
      virt_pos_midpoint(&p1_e, &p1_e_sec, &contact_point);
    }
    else {
      virt_pos_midpoint(&p2_e, &p2_e_sec, &contact_point);
    }
    ret = 1;
  }
  else if (d1 < thresh || d2 < thresh) {
    if (d1 > d2) {
      contact_point = p1_e;
    }
    else {
      contact_point = p2_e;
    }
    ret = 2;
  }
  else {
    //solution failed to find a contact point
    ret = -1;
  }
  *result = contact_point;
  return ret;
}


void tmi_points_of_polygon(polygon* check,vector_2* line, virt_pos* min_point, virt_pos* max_point, virt_pos* second_min_point, virt_pos* second_max_point) {
  double min = 0, max = 0, temp = 0;
  virt_pos a_point;
  virt_pos prim_min_point = *zero_pos, sec_min_point = *zero_pos;
  virt_pos prim_max_point = *zero_pos, sec_max_point = *zero_pos;
  double sec_min_mag = 0, sec_max_mag = 0;
  int sec_min_set = 0;
  int sec_max_set = 0;
  
  for (int i = 0; i < check->sides; i++) {
    get_actual_point(check,i, &a_point);
    
    temp = get_projected_length(&a_point, line);
    if (i == 0 ) {
      prim_min_point = a_point;
      prim_max_point = a_point;
      max = temp;
      min = temp;
      
    }
    else {
      if (temp > max) {
	sec_max_mag = max;
	sec_max_point = prim_max_point;
	sec_max_set = 1;
	max = temp;
	prim_max_point = a_point;
      }
      else if (!sec_max_set || temp > sec_max_mag) {
	sec_max_mag = temp;
	sec_max_point = a_point;
	sec_max_set = 1;
      }
      if (temp < min) {
	sec_min_mag = min;
	sec_min_point = prim_min_point;
	sec_min_set = 1;
	min = temp;
        prim_min_point = a_point;
      }
      else if (!sec_min_set || temp < sec_min_mag) {
	sec_min_mag = temp;
	sec_min_point = a_point;
	sec_min_set = 1;
      }

    }
  }
  *min_point = prim_min_point;
  *max_point = prim_max_point;
  *second_min_point = sec_min_point;
  *second_max_point = sec_max_point;

}

void extreme_projections_of_polygon(polygon* check,virt_pos* new_origin,vector_2* line, double* min_result, double* max_result) {
  double min = 0, max = 0, temp = 0;
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
  double projectedLength = 0;
  if (!isZeroVec(line)) {
    float delta = atan2(vec->v2, vec->v1) - atan2(line->v2, line->v1);
    double hyp = vector_2_magnitude(vec);
    projectedLength = cos(delta) * hyp;
  }
  return projectedLength;
}

void get_parallell_comp(vector_2* vec, vector_2* line, vector_2* result) {
  double mag = get_projected_length_vec(vec, line);
  vector_2 unit;
  make_unit_vector(line, &unit);
  vector_2_scale(&unit, mag, result);
  
}

void get_orthogonal_comp(vector_2* vec, vector_2* line, vector_2* result) {
  vector_2 parallell;
  get_parallell_comp(vec, line, &parallell);
  vector_2_sub(vec, &parallell, result);
}

void decompose_vector(vector_2* vec, vector_2* line, vector_2* p, vector_2* o) {
  get_parallell_comp(vec, line, p);
  vector_2_sub(vec, p, o);
}

void  project_point_onto_line(virt_pos* point, vector_2* line, virt_pos* result) {
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
  return get_projected_length_pos(point, line);
}

double get_projected_length_pos(virt_pos* point, vector_2* line) {
  double projectedLength = 0;
  
  if (!isZeroVec(line)) {
    float delta = atan2(point->y, point->x) - atan2(line->v2, line->v1);
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
  double mag;
  mag = vector_2_magnitude(op);
  if (mag != 0) {
    result->v1 = op->v1 / mag;
    result->v2 = op->v2 / mag;
    assert(is_a_unit_vector(result) == 1);
  }
  else {
    fprintf(stderr, "attempting to make a zero vector a unit vector\n");
    exit(3);
  }
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

void virt_pos_scale(virt_pos* p1, double c, virt_pos* result) {
  result->x = round(p1->x * c);
  result->y = round(p1->y * c);
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

void vector_2_to_virt_pos_ceil(vector_2* in, virt_pos* out) {
  out->x = ceil(in->v1);
  out->y = ceil(in->v2);
}

void vector_2_to_virt_pos_zero(vector_2* in, virt_pos* out) {
  out->x = (int)in->v1;
  out->y = (int)in->v2;
}

//points from p1 to p2
vector_2 vector_between_points( virt_pos* p1, virt_pos* p2) {
  vector_2 result;
  result.v1 = p2->x - p1->x;
  result.v2 = p2->y - p1->y;
  return result;
}

void print_vector(vector_2* vec) {
  fprintf(stdout, "vector is %f, %f\n", vec->v1, vec->v2);
}

void print_point(virt_pos* pos) {
  fprintf(stdout, "point is x:%d, y:%d\n", pos->x, pos->y);
}

int sign_of(double val) {
  if (val == 0) {
    return 0;
  }
  else if (val < 0) {
    return -1;
  }
  else {
    return 1;
  }
  
}

//clamps angle between 0 and 2pi
double clamp_rotation(double ang) {
  double r = 0;
  double s = 0;
  if (ang > 2 * M_PI || ang < 0) {
    r = (ang / (2 * M_PI));
    if (ang > 2 * M_PI) {
      s = r - (int)r;
    }
    else {
      r *= -1;
      s = 1 - (r - (int)r);
    }
    ang = 2 * M_PI * s;
  }
  return ang;
}

double angle_of_vector(vector_2* vec) {
  return atan2(vec->v2, vec->v1);
}

//from d1 to d2, in radians
double difference_of_radians(double r1, double r2) {
  double diff = clamp_rotation(r2 - r1);
  if (diff > (M_PI)) {
    diff = -1 * (2 * M_PI - diff);
  }
  return diff;
}

void add_offset_to_center(polygon* p, virt_pos* off) {
  virt_pos_add(p->center, off, p->center);
  recalc_corners(p);
}

void free_polygon_center(polygon* p) {
  free(p->center);
}

//needed to have camera track the user, and also for tethers
//does not actually enforce read_only
virt_pos* read_only_polygon_center(polygon* p) {
  return p->center;
  
}

ad_vec* create_ad_vec(double alpha) {
  ad_vec* new = malloc(sizeof(ad_vec));
  init_ad_vec(new, zero_vec, alpha);
  return new;
}

void init_ad_vec(ad_vec* v, vector_2* v_set, double alpha_set) {
  v->add = *zero_vec;
  v->vec = *v_set;
  v->alpha = alpha_set;
}

void free_ad_vec(ad_vec* rm) {
  free(rm);
}

void add_to_ad_vec(ad_vec* d, vector_2* v) {
  vector_2_add(&d->add, v, &d->add);
}

void calc_ad_vec(ad_vec* d) {
  exponential_decay_vector(&d->vec, &d->add, &d->vec, d->alpha);
  d->add = *zero_vec;
}

void exponential_decay_vector(vector_2* old, vector_2* cur, vector_2* new, double alpha) {
  exponential_decay(old->v1, cur->v1, &(new->v1), alpha);
  exponential_decay(old->v2, cur->v2, &(new->v2), alpha);
}

void exponential_decay(double old, double cur, double* new, double alpha) {
  *new = alpha * cur + (1 - alpha) * old;
}

void timed_exponential_decay_vector(vector_2* old, vector_2* cur, vector_2* new, double alpha, double dt_scale) {
  timed_exponential_decay(old->v1, cur->v1, &(new->v1), alpha, dt_scale);
  timed_exponential_decay(old->v2, cur->v2, &(new->v2), alpha, dt_scale);
}

//dt_scale is, given a framerate r, get_dT() * r
void timed_exponential_decay(double old, double cur, double* new, double alpha, double dt_scale) {
  alpha = 1 - alpha;
  double old_alpha = pow(alpha, dt_scale);
  double cur_alpha = 1 - old_alpha;
  *new = cur_alpha * cur + old_alpha * old;
}

void timed_calc_ad_vec(ad_vec* d, double dt_scale) {
  timed_exponential_decay_vector(&d->vec, &d->add, &d->vec, d->alpha, dt_scale);
  d->add = *zero_vec;
}


vector_2 get_ad_vec(ad_vec* v) {
  return v->vec;
}
