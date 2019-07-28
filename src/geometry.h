#ifndef FILE_GEOMETRY_SEEN
#define FILE_GEOMETRY_SEEN

#define RAD_2_DEG 180 / M_PI
#define DEG_2_RAD M_PI / 180

typedef
struct {
  int x;
  int y;
} virt_pos;
//hardcode 2dim vector, because all my code only works with 2d and this allows for vectors to be on stack
typedef
struct {
  double v1;
  double v2;
} vector_2;

typedef
struct {
  int sides;
  double scale;
  double rotation;
  virt_pos* center;
  //base corners, which contain relative shape of object but no rotation/scale
  virt_pos* base_corners;
  //rotated corners, to store rotated posistions of corners to cut back calculations
  virt_pos* corners;
  //normals, haven't considered much about them
  //having them be unit vectors would be nice
  //otherwise some way of determining which normal corresponds to which side or pair of corners
  vector_2* base_normals; 
  vector_2* normals;
  virt_pos rotation_offset;
} polygon;

extern vector_2* zero_vec;

extern virt_pos* zero_pos;

extern vector_2* x_axis;

extern vector_2* y_axis;

polygon* createPolygon(int sides);
polygon* clonePolygon(polygon* src);
polygon* createNormalPolygon(int sides);
polygon* createRectangle(int width, int height);

void freePolygon(polygon* poly);

void generate_normals_for_polygon(polygon* poly);

void make_normal_polygon(polygon* poly);
void make_square(polygon* poly);

void stretch_deform_vert(polygon* poly, double amount);
void stretch_deform_horz(polygon* poly, double amount);




virt_pos* get_center(polygon* poly);
void set_center(polygon* poly, virt_pos* val);

void set_rotation(polygon* poly, double new);
double get_rotation(polygon* poly);
void set_rotation_offset(polygon* poly, virt_pos* offset);

void get_actual_point(polygon* poly, int i, virt_pos* result);
void get_base_point(polygon* poly, int i, virt_pos* result);

void set_base_point(polygon* poly, int i, virt_pos* set);
void get_actual_normal(polygon* poly, int i, vector_2* result);

int do_polygons_intersect(polygon* p1, polygon* p2);
int find_mtv_of_polygons(polygon* p1, polygon* p2, vector_2* mtv);
void extreme_projections_of_polygon(polygon* check,virt_pos* new_origin,vector_2* line, double* min_result, double* max_result);

void decompose_vector(vector_2* vec, vector_2* line, vector_2* p, vector_2* o);
void project_point_onto_line(virt_pos* point, vector_2* line, virt_pos* result);
double get_projected_length(virt_pos* point, vector_2* line);
double get_projected_length_vec(vector_2* vec, vector_2* line);
double get_projected_length_pos(virt_pos* point, vector_2* line);

double distance_from_origin(virt_pos* point);
double distance_between_points(virt_pos* p1, virt_pos* p2);

int is_a_unit_vector(vector_2* vec);
int isZeroPos(virt_pos* pos);
int isZeroVec(vector_2* vec);
int isCloseEnoughToZeroVec(vector_2* vec);

void make_unit_vector(vector_2* op, vector_2* result);

void vector_2_add(vector_2* x, vector_2* y, vector_2* result);
void vector_2_sub(vector_2* x, vector_2* y, vector_2* result);
void vector_2_mul(vector_2* x, vector_2* y, vector_2* result);
void vector_2_div(vector_2* x, vector_2* y, vector_2* result);
void vector_2_scale(vector_2* v, double c, vector_2* result);
double vector_2_magnitude(vector_2* v);
void vector_2_rotate(vector_2* vec, double radians, vector_2* result);

void virt_pos_add(virt_pos* p1, virt_pos* p2, virt_pos* result);
void virt_pos_sub(virt_pos* p1, virt_pos* p2, virt_pos* result);
void virt_pos_scale(virt_pos* p1, double c, virt_pos* result);
void virt_pos_midpoint(virt_pos* p1, virt_pos* p2, virt_pos* result);
void virt_pos_rotate(virt_pos* pos, double radians, virt_pos* result);

void virt_pos_to_vector_2(virt_pos* in, vector_2* out);
void vector_2_to_virt_pos(vector_2* in, virt_pos* out);
void vector_2_to_virt_pos_ceil(vector_2* in, virt_pos* out);
void vector_2_to_virt_pos_zero(vector_2* in, virt_pos* out);

void vector_between_points( virt_pos* p1, virt_pos* p2, vector_2* result);

void print_vector(vector_2* vec);
void print_point(virt_pos* pos);

void exponential_decay_vector(vector_2* old, vector_2* cur, vector_2* new, double alpha);
void exponential_decay(double old, double cur, double* new, double alpha);

int sign_of(double val);

#endif
