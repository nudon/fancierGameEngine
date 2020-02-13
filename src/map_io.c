#include <stdio.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include "map_io.h"
#include "objects.h"

map* xml_read_map(xmlNodePtr map_node);

void test_xml_parse();
void test_plane_parse(plane* plane);

void xml_write_load_zone(FILE* file_out, load_zone* lz);
load_zone* xml_read_load_zone(xmlNodePtr lz_node);

void xml_write_plane(FILE* file_out, plane* plane);
plane* xml_read_plane(xmlNodePtr plane_node);

void xml_write_event(FILE* file_out, event* e);
event* xml_read_event(xmlNodePtr event_node);

void xml_write_smarts(FILE* file_out, smarts* sm);
smarts* xml_read_smarts(xmlNodePtr smarts_node);

void xml_write_spawner(FILE* file_out, compound_spawner* spawn);
compound_spawner* xml_read_spawner(xmlNodePtr spawn_node);

void xml_write_compound(FILE* file_out, compound* comp);
compound* xml_read_compound(xmlNodePtr comp_node);

void xml_write_body(FILE* file_out, body* body);
body* xml_read_body(xmlNodePtr body_node);

void xml_write_fizzle(FILE* file_out, fizzle* fizz);
fizzle* xml_read_fizzle(xmlNodePtr fizzle_node);

void xml_write_polygon(FILE* file_out, polygon* poly);
polygon* xml_read_polygon(xmlNodePtr polygon_node);

void xml_write_picture(FILE* file_out, picture* pic);
picture* xml_read_picture(xmlNodePtr pic_node);

void xml_write_vector_2(FILE* file_out, vector_2* vec, char* id);
void xml_read_vector_2(xmlNodePtr vec_node, vector_2* result);

void xml_write_virt_pos(FILE* file_out, virt_pos* vp, char* id);
void xml_read_virt_pos(xmlNodePtr vp_node, virt_pos* result);


void xml_write_attributes(FILE* file_out, att* atts);
att* xml_read_attributes(xmlNodePtr atts);
	    
double get_double_prop(xmlNodePtr node, char* id);
int get_int_prop(xmlNodePtr node, char* id);
char* get_charp_prop(xmlNodePtr node, char* id);

xmlNodePtr get_child_by_name(xmlNodePtr parent, char* name);
xmlNodePtr get_child_by_name_and_id(xmlNodePtr parent, char* name, char* id);


map* load_map(char* filename) {
  xmlDocPtr doc = xmlParseFile(filename);
  xmlNodePtr root = xmlDocGetRootElement(doc);
  map* the_map = xml_read_map(root);
  return the_map;
}

void xml_write_map(FILE* file_out, map* map) {
  prep_for_save(map);
  fprintf(file_out, "<map id=\"%s\">\n", get_map_name(map));
  gen_node* curr = get_planes(map)->start;
  plane* aPlane = NULL;
  while(curr != NULL) {
    aPlane = (plane*)curr->stored;
    xml_write_plane(file_out, aPlane);
    curr = curr->next;
  }
  fprintf(file_out, "</map>\n");
}

map* xml_read_map(xmlNodePtr map_node) {
  xmlNodePtr child = map_node->xmlChildrenNode;
  char* text = NULL;
  text = (char*)xmlGetProp(map_node, (const xmlChar*)"id");
  map* map = create_map(text);
  plane* aPlane = NULL;
  free(text);
  while(child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar*)"plane") == 0) {
      aPlane = xml_read_plane(child);
      add_plane(map, aPlane);
    }
    child = child->next;
  }
  return map;
}


void xml_write_plane(FILE* file_out, plane* plane) {
  box mat_dim = get_dim_shape(get_shm(plane));
  box cell_dim = get_cell_shape(get_shm(plane));
  fprintf(file_out, "<plane rows=\"%d\" cols=\"%d\" cellH=\"%d\" cellW=\"%d\" zLevel=\"%f\" id=\"%s\">\n", mat_dim.height, mat_dim.width, cell_dim.height, cell_dim.width , get_z_level(plane), get_plane_name(plane));
  gen_node* cur_comp = get_compounds(plane)->end;
  while (cur_comp != NULL) {
    xml_write_compound(file_out, (compound*)cur_comp->stored);
    cur_comp = cur_comp->prev;
  }
  gen_node* curr_load = get_load_zones(plane)->end;
  while (curr_load != NULL) {
    xml_write_load_zone(file_out, (load_zone*)curr_load->stored);
    curr_load = curr_load->prev;
  }
  gen_node* curr_spawner = get_spawners(plane)->end;
  while (curr_spawner != NULL) {
    xml_write_spawner(file_out, (compound_spawner*)curr_spawner->stored);
    curr_spawner = curr_spawner->prev;
  }
  fprintf(file_out, "</plane>\n");
}

plane* xml_read_plane(xmlNodePtr plane_node) {
  xmlNodePtr child = plane_node->xmlChildrenNode;
  int w = -1,h = -1,r = -1,c = -1, z = -1;
  char* text = NULL;
  r = get_int_prop(plane_node, "rows");
  c = get_int_prop(plane_node, "cols");
  w = get_int_prop(plane_node, "cellW");
  h = get_int_prop(plane_node, "cellH");
  z = get_int_prop(plane_node, "zLevel");
  
  text = (char*)xmlGetProp(plane_node, (const xmlChar*)"id");
  spatial_hash_map* map = create_shm(w,h,c,r);
  plane* plane = create_plane(map, text);
  free(text);
  set_z_level(plane, z);
  compound* comp = NULL;
  load_zone* lz = NULL;
  compound_spawner* spawn = NULL;
  while (child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar*)"compound") == 0) {
      comp = xml_read_compound(child);
      add_compound_to_plane(plane, comp);
    }
    else if (xmlStrcmp(child->name, (const xmlChar*)"load_zone") == 0) {
      lz = xml_read_load_zone(child);
      add_load_zone_to_plane(plane, lz);
    }
    else if (xmlStrcmp(child->name, (const xmlChar*)"spawner") == 0) {
      spawn = xml_read_spawner(child);
      add_spawner_to_plane(plane, spawn);
    }
    
    child = child->next;
  }
  return plane;
}

void xml_write_load_zone(FILE* file_out, load_zone* lz) {
  char* fm = get_lz_from_map(lz);
  char* tm = get_lz_to_map(lz);
  char* fp = get_lz_from_plane(lz);
  char* tp = get_lz_to_plane(lz);
  fprintf(file_out, "<load_zone from_map=\"%s\" to_map=\"%s\" from_plane=\"%s\" to_plane=\"%s\" >\n",
	  fm, tm, fp, tp);
  virt_pos vp = get_lz_dest(lz);
  xml_write_virt_pos(file_out, &vp, "dest");
  xml_write_event(file_out, get_lz_event(lz));
  fprintf(file_out, "</load_zone >\n");
}

load_zone* xml_read_load_zone(xmlNodePtr lz_node) {
  char* fm = NULL;
  char* tm = NULL;
  char* fp = NULL;
  char* tp = NULL;
  virt_pos vp = *zero_pos;
  event* e = NULL;
  
  fm = get_charp_prop(lz_node, "from_map");
  tm = get_charp_prop(lz_node, "to_map");
  fp = get_charp_prop(lz_node, "from_plane");
  tp = get_charp_prop(lz_node, "to_plane");
  xmlNodePtr child = lz_node->xmlChildrenNode;
  child = get_child_by_name_and_id(lz_node, "virt_pos", "dest");
  xml_read_virt_pos(child, &vp);
  child = get_child_by_name(lz_node, "event");
  e = xml_read_event(child);
  load_zone* lz = make_load_zone(fm, tm, fp, tp, &vp, e);
  free(fm);
  free(tm);
  free(fp);
  free(tp);
  return lz;
}


//only saves static/non moving events
void xml_write_event(FILE* file_out, event* e) {
  fprintf(file_out, "<event func=\"%s\" >\n", get_event_name(e));
  xml_write_polygon(file_out, get_polygon(get_event_collider(e)));
  fprintf(file_out, "</event >\n");
}

event* xml_read_event(xmlNodePtr event_node) {
  char* text = NULL;
  polygon* poly = NULL;
  text = get_charp_prop(event_node, "func");
  xmlNodePtr child = get_child_by_name(event_node, "polygon");
  poly = xml_read_polygon(child);
  event* e = make_event(poly);
  set_event_by_name(e, text);
  free(text);
  return e;
}

void xml_write_spawner(FILE* file_out, compound_spawner* spawn) {
  fprintf(file_out, "<spawner compound_name=\"%s\" spawn_cap = \"%i\">\n", get_spawner_name(spawn), get_spawner_cap(spawn));
  virt_pos temp;
  get_spawner_pos(spawn, &temp);
  xml_write_virt_pos(file_out, &temp, "spawn_center");
  fprintf(file_out, "</spawner>\n");
}

compound_spawner* xml_read_spawner(xmlNodePtr spawn_node) {
  char* spawn_name = get_charp_prop(spawn_node, "compound_name");
  int cap = get_int_prop(spawn_node, "spawn_cap");
  virt_pos spawn_pos = *zero_pos;
  xmlNodePtr child = get_child_by_name(spawn_node, "virt_pos");
  xml_read_virt_pos(child, &spawn_pos);
  compound_spawner* spawn = create_compound_spawner(spawn_name, cap, spawn_pos.x, spawn_pos.y);
  free(spawn_name);
  return spawn;
}

void xml_write_compound(FILE* file_out, compound* comp) {
  if (get_spawner_p(comp) != NULL) {
    return;
  }
  fprintf(file_out, "<compound>\n");
  gen_node* cur = get_bodies(comp)->start;
  while(cur != NULL) {
    xml_write_body(file_out, (body*)cur->stored);
    cur = cur->next;
  }
  xml_write_smarts(file_out, get_compound_smarts(comp));
  fprintf(file_out, "</compound>\n");
}

compound* xml_read_compound(xmlNodePtr comp_node) {
  compound* comp = create_compound();
  xmlNodePtr child = NULL;
  body* cur_body = NULL;
  smarts* sm = NULL;
  child = get_child_by_name(comp_node, "smarts");
  sm = xml_read_smarts(child);
  if (sm != NULL) {
    set_compound_smarts(comp, sm);
    add_smarts_to_comp(comp);
  }
  child = comp_node->xmlChildrenNode;
  while(child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar*)"body") == 0) {
      cur_body = xml_read_body(child);
      add_body_to_compound(comp, cur_body);
    }
    child = child->next;
  }
  return comp;
}

void xml_write_body(FILE* file_out, body* body) {
  fprintf(file_out, "<body poltergeist=\"%s\" >\n", get_polt_name(get_poltergeist(body)));
  xml_write_fizzle(file_out, get_fizzle(body));
  xml_write_polygon(file_out, get_polygon(get_collider(body)));
  xml_write_picture(file_out, get_picture(body));
  gen_node* curr = get_body_events(body)->start;
  event* e = NULL;
  while(curr != NULL) {
    e = (event*)curr->stored;
    xml_write_event(file_out, e);
    curr = curr->next;
  }
  xml_write_smarts(file_out, get_body_smarts(body));
  fprintf(file_out, "</body>\n");
}

body* xml_read_body(xmlNodePtr body_node) {
  body* body = NULL;
  fizzle* fizz = NULL;
  collider* coll = NULL;
  polygon* poly = NULL;
  picture* pic = NULL;
  gen_list* temp_event_list = create_gen_list();
  event* e = NULL;
  poltergeist* polt = NULL;
  char* text = (char*)xmlGetProp(body_node, (const xmlChar*)"poltergeist");
  if (strcmp(text, "NULL") != 0) {
    polt = make_poltergeist();
    set_polt_by_name(polt, text);
  }
  free(text);
  xmlNodePtr child = get_child_by_name(body_node, "smarts");
  smarts* sm = xml_read_smarts(child);
  child = get_child_by_name(body_node, "fizzle");
  fizz = xml_read_fizzle(child);
  child = get_child_by_name(body_node, "polygon");
  poly = xml_read_polygon(child);
  coll = make_collider_from_polygon(poly);
  child = get_child_by_name(body_node, "picture");
  pic = xml_read_picture(child);
  child = body_node->xmlChildrenNode;
  
  while(child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar*)"event") == 0) {
      e = xml_read_event(child);
      list_prepend(temp_event_list, create_gen_node(e));
    }
    child = child->next;
  }
  body = createBody(fizz, coll);
  set_picture(body, pic);
  set_poltergeist(body, polt);
  set_body_smarts(body, sm);
  gen_node* curr = temp_event_list->start;
  while(curr != NULL) {
    e = (event*)curr->stored;
    add_event_to_body(body, e);
    curr = curr->next;
  }
  free_gen_list(temp_event_list);
  return body;
}

void xml_write_fizzle(FILE* file_out, fizzle* fizz) {
  fprintf(file_out, "<fizzle mass=\"%f\" moi=\"%f\" rot_accel=\"%f\" rot_vel=\"%f\" bounce=\"%f\" >\n", fizz->mass, fizz->moi, fizz->rot_acceleration, fizz->rot_velocity, get_bounce(fizz));
  xml_write_vector_2(file_out, &(fizz->velocity), "velocity");
  xml_write_vector_2(file_out, &(fizz->dampening), "dampening");
  xml_write_vector_2(file_out, &(fizz->impact), "impact");
  xml_write_vector_2(file_out, &(fizz->gravity), "gravity");
  xml_write_vector_2(file_out, &(fizz->tether), "tether");
  xml_write_vector_2(file_out, &(fizz->net_acceleration), "net_acceleration");

  fprintf(file_out, "</fizzle>\n");
}

fizzle* xml_read_fizzle(xmlNodePtr fizzle_node) {
  fizzle* fizz = createFizzle();
  init_fizzle(fizz);
  double val = -1;
  xmlChar* text = NULL;
  vector_2 vec;

  val = get_double_prop(fizzle_node, "mass");
  set_mass(fizz, val);

  val = get_double_prop(fizzle_node, "moi");
  set_moi(fizz, val);

  val = get_double_prop(fizzle_node, "rot_accel");
  fizz->rot_acceleration = val;

  val = get_double_prop(fizzle_node, "rot_vel");
  fizz->rot_velocity = val;

  val = get_double_prop(fizzle_node, "bounce");
  set_bounce(fizz, val);
  
  xmlNodePtr child = fizzle_node->xmlChildrenNode;
  
  while(child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar*)"vector_2") == 0) {
      xml_read_vector_2(child, &vec);
      text = xmlGetProp(child, (const xmlChar*)"id");
      if (xmlStrcmp(text, (const xmlChar*)"velocity") == 0) {
	fizz->velocity = vec;
      }
      else if (xmlStrcmp(text, (const xmlChar*)"dampening") == 0) {
	fizz->dampening = vec;
      }
      else if (xmlStrcmp(text, (const xmlChar*)"impact") == 0) {
	fizz->impact = vec;
      }
      else if (xmlStrcmp(text, (const xmlChar*)"gravity") == 0) {
	fizz->gravity = vec;
      }
      else if (xmlStrcmp(text, (const xmlChar*)"tether") == 0) {
	fizz->tether = vec;
      }
      else if (xmlStrcmp(text, (const xmlChar*)"net_acceleration") == 0) {
	fizz->net_acceleration = vec;
      }
      free(text);
    }
    child = child->next;
  }
  return fizz;
}

void xml_write_polygon(FILE* file_out, polygon* poly) {
  int sides = get_sides(poly);
  fprintf(file_out, "<polygon sides=\"%d\" scale=\"%f\" rotation=\"%f\">\n", sides, get_scale(poly), get_rotation(poly));
  virt_pos pc = get_center(poly);
  xml_write_virt_pos(file_out, &pc, "center");
  int buff_size = 16;
  char buff[buff_size];
  virt_pos temp = *zero_pos;
  for (int i = 0; i < sides; i++) {
    get_base_point(poly, i, &temp);
    snprintf(buff, buff_size, "%d", i);
    xml_write_virt_pos(file_out, &temp, buff);
  }
  fprintf(file_out, "</polygon>\n");
}

polygon* xml_read_polygon(xmlNodePtr polygon_node) {
  polygon* poly = NULL;
  xmlChar* text = NULL;
  double val = 0;
  int ival = 0;

  int sides = get_int_prop(polygon_node, "sides");
  poly = createPolygon(sides);
  
  double scale = get_double_prop(polygon_node, "scale");

  val = get_double_prop(polygon_node, "rotation");
  set_rotation(poly, val);
  
  xmlNodePtr child = polygon_node->xmlChildrenNode;
  virt_pos vp;
  while(child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar*)"virt_pos") == 0) {
      xml_read_virt_pos(child, &vp);
      text = xmlGetProp(child, (const xmlChar*)"id");
      if (xmlStrcmp(text, (const xmlChar*)"center") == 0) {
	set_center(poly, &vp);
      }
      else {
	ival = atoi((const char*)text);
	if (val >= sides) {
	  fprintf(stderr, "xml file contained a corner indexed beyond polygons sides\n");
	}
	set_base_point(poly, ival, &vp);
      }
      free(text);
    }
    child = child->next;
  }
  generate_normals_for_polygon(poly);
  set_scale(poly, scale);
  recalc_corners_and_norms(poly);
  return poly;
}

void xml_write_picture(FILE* file_out, picture* pic) {
  if (pic != NULL) {
    fprintf(file_out, "<picture file_name=\"%s\" />\n", pic->file_name);
  }
}

picture* xml_read_picture(xmlNodePtr pic_node) {
  char* fn = (char*)xmlGetProp(pic_node, (const xmlChar*)"file_name");
  picture* pic = make_picture(fn);
  free(fn);
  return pic;
}

void xml_write_smarts(FILE* file_out, smarts* sm) {
  if (sm == NULL) {
    return;
  }
  if (get_smarts_body(sm) != NULL) {
    //write body mem, stats and atts
    fprintf(file_out, "<smarts type=\"body\">\n");
    xml_write_attributes(file_out, get_body_attributes(sm));
    fprintf(file_out, "</smarts>\n");
  }
  else if (get_smarts_compound(sm) != NULL) {
    //write compound mem, stats and atts
    fprintf(file_out, "<smarts type=\"compound\">\n");
    xml_write_attributes(file_out, get_comp_attributes(sm));
    fprintf(file_out, "</smarts>\n");
  }
}

smarts* xml_read_smarts(xmlNodePtr smarts_node) {
  if (smarts_node == NULL) {
    return NULL;
  }
  xmlNodePtr child = get_child_by_name(smarts_node, "decision_att");
  char* text = get_charp_prop(smarts_node, "type");
  smarts* sm = NULL;
  if (strcmp(text, "body") == 0) {
    att* b_atts = xml_read_attributes(child);
    sm = make_smarts();
    set_body_attributes(sm, b_atts);
  }
  else if (strcmp(text, "compound") == 0) {
    att* c_atts = xml_read_attributes(child);
    sm = make_smarts();
    set_comp_attributes(sm, c_atts);
  }
  free(text);
  return sm;
}


void xml_write_ad_vec(FILE* file_out, ad_vec* vec, char* id) {
  fprintf(file_out, "<ad_vec id=\"%s\" alpha=\"%f\" >\n", id, vec->alpha);
  xml_write_vector_2(file_out, &vec->vec, "vec");
  fprintf(file_out, "</ad_vec>\n");
}

void xml_read_ad_vec(xmlNodePtr node, ad_vec* result) {
  double alpha = get_double_prop(node, "alpha");
  xmlNodePtr vector_node = get_child_by_name(node, "vector_2");
  vector_2 temp = *zero_vec;
  xml_read_vector_2(vector_node, &temp);
  result->alpha = alpha;
  result->vec = temp;
}


void xml_write_vector_2(FILE* file_out, vector_2* vec, char* id) {
  fprintf(file_out, "<vector_2 id=\"%s\" v1=\"%f\" v2=\"%f\" />\n", id, vec->v1, vec->v2);
}

void xml_read_vector_2(xmlNodePtr vec_node, vector_2* result) {
  double val = 0;

  val = get_double_prop(vec_node, "v1");
  result->v1 = val;

  val = get_double_prop(vec_node, "v2");
  result->v2 = val;
}

void xml_write_virt_pos(FILE* file_out, virt_pos* vp, char* id) {
  fprintf(file_out, "<virt_pos id=\"%s\" x=\"%d\" y=\"%d\" />\n", id, vp->x, vp->y);
}

void xml_read_virt_pos(xmlNodePtr vp_node, virt_pos* result) {
  int ival = 0;

  ival = get_int_prop(vp_node, "x");
  result->x = ival;

  ival = get_int_prop(vp_node, "y");
  result->y = ival;
}

void xml_write_attributes(FILE* file_out, att* atts) {
  char* text = atts_to_text(atts);
  fprintf(file_out, "<decision_att bits=\"%s\" />\n", text);
  free(text);
}

att* xml_read_attributes(xmlNodePtr atts_node) {
  char* text = (char*)xmlGetProp(atts_node, (const xmlChar*)"bits");
  att* atts = text_to_atts(text);
  free(text);
  return atts;
}

int get_int_prop(xmlNodePtr node, char* id) {
  xmlChar* text_val = xmlGetProp(node, (const xmlChar*)id);
  int val = -1;
  val = atoi((const char*)text_val);
  free(text_val);
  return val;
}

double get_double_prop(xmlNodePtr node, char* id) {
  xmlChar* text_val = xmlGetProp(node, (const xmlChar*)id);
  double val = -1;
  val = atof((const char*)text_val);
  free(text_val);
  return val;
}

char* get_charp_prop(xmlNodePtr node, char* id) {
  xmlChar* text_val = xmlGetProp(node, (const xmlChar*)id);
  return (char*)text_val;
}

xmlNodePtr get_child_by_name(xmlNodePtr parent, char* name) {
  xmlNodePtr child = parent->xmlChildrenNode;
  while(child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar*)name) == 0) {
      return child;
    }
    child = child->next;
  }
  return NULL;
}

xmlNodePtr get_child_by_name_and_id(xmlNodePtr parent, char* name, char* id) {
  xmlNodePtr child = parent->xmlChildrenNode;
  char* temp = NULL;
  while(child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar*)name) == 0) {
      temp = get_charp_prop(child, "id");
      if (strcmp(temp, id) == 0) {
	free(temp);
	return child;
      }
      free(temp);
      temp = NULL;
    }
    child = child->next;
  }
  return NULL;
}
