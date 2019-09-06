#include <stdio.h>
#include <stdlib.h>
#include <libxml/parser.h>
#include "map_io.h"

map* xml_read_map(xmlNodePtr map_node);

void test_xml_parse();
void test_plane_parse(plane* plane);

void xml_write_load_zone(FILE* file_out, load_zone* lz);
load_zone* xml_read_load_zone(xmlNodePtr lz_node);

void xml_write_plane(FILE* file_out, plane* plane);
plane* xml_read_plane(xmlNodePtr plane_node);

void xml_write_event(FILE* file_out, event* e);
event* xml_read_event(xmlNodePtr event_node);

void xml_write_gi(FILE* file_out, gi* g);
gi* xml_read_gi(xmlNodePtr gi_node);

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

void xml_write_vector_2(FILE* file_out, vector_2* vec, char* name);
void xml_read_vector_2(xmlNodePtr vec_node, vector_2* result);

void xml_write_virt_pos(FILE* file_out, virt_pos* vp, char* name);
void xml_read_virt_pos(xmlNodePtr vp_node, virt_pos* result);


void xml_write_attributes(FILE* file_out, decision_att* atts);
decision_att* xml_read_attributes(xmlNodePtr atts);						    
		    
double get_double_prop(xmlNodePtr node, char* id);
int get_int_prop(xmlNodePtr node, char* id);
char* get_charp_prop(xmlNodePtr node, char* id);

map* load_map(char* filename) {
  xmlDocPtr doc = xmlParseFile(filename);
  xmlNodePtr root = xmlDocGetRootElement(doc);
  map* the_map = xml_read_map(root);
  return the_map;
}

void xml_write_map(FILE* file_out, map* map) {
  fprintf(file_out, "<map name=\"%s\">\n", get_map_name(map));
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
  text = (char*)xmlGetProp(map_node, (const xmlChar*)"name");
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
  fprintf(file_out, "<plane rows=\"%d\" cols=\"%d\" cellH=\"%d\" cellW=\"%d\" zLevel=\"%f\" name=\"%s\">\n", mat_dim.height, mat_dim.width, cell_dim.height, cell_dim.width , get_z_level(plane), get_plane_name(plane));
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
  
  text = (char*)xmlGetProp(plane_node, (const xmlChar*)"name");
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
  char* text = NULL;
  virt_pos vp = *zero_pos;
  event* e = NULL;
  
  fm = (char*)xmlGetProp(lz_node, (const xmlChar*)"from_map");
  tm = (char*)xmlGetProp(lz_node, (const xmlChar*)"to_map");
  fp = (char*)xmlGetProp(lz_node, (const xmlChar*)"from_plane");
  tp = (char*)xmlGetProp(lz_node, (const xmlChar*)"to_plane");
  xmlNodePtr child = lz_node->xmlChildrenNode;

  while(child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar*)"virt_pos") == 0) {
      text = (char*)xmlGetProp(child, (const xmlChar*)"name");
      if (strcmp(text, "dest") == 0) {
	xml_read_virt_pos(child, &vp);
      }
      free(text);
    }
    else if (xmlStrcmp(child->name, (const xmlChar*)"event") == 0) {
      e = xml_read_event(child);
    }
    child = child->next;
  }
  load_zone* lz = make_load_zone(fm, tm, fp, tp, &vp, e);
  free(fm);
  free(tm);
  free(fp);
  free(tp);
  return lz;
}


void xml_write_event(FILE* file_out, event* e) {
  fprintf(file_out, "<event func=\"%s\" >\n", get_event_name(e));
  xml_write_polygon(file_out, get_polygon(get_event_collider(e)));
  //ignoring the attached bodies because it's hard to coordinate shared data
  //have that handled by a special routine or somethings
  fprintf(file_out, "</event >\n");
}

event* xml_read_event(xmlNodePtr event_node) {
  char* text = NULL;
  polygon* poly = NULL;
  text = (char*)xmlGetProp(event_node, (const xmlChar*)"func");
  xmlNodePtr child = event_node->xmlChildrenNode;
  while(child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar*)"polygon") == 0) {
      poly = xml_read_polygon(child);
    }
    child = child->next;
  }
  event* e = make_event(poly);
  set_event_by_name(e, text);
  free(text);
  return e;
}

void xml_write_gi(FILE* file_out, gi* g) {
  fprintf(file_out, "<gi decay_alpha=\"%f\">\n", get_exp_decay_alpha(g));
  vector_2 temp = get_curr_dir(g);
  xml_write_vector_2(file_out, &temp, "curr_dir");
  temp = get_new_dir(g);
  xml_write_vector_2(file_out, &temp, "new_dir");
  xml_write_attributes(file_out, get_gi_attributes(g));  
  fprintf(file_out, "</gi>\n");
}

gi* xml_read_gi(xmlNodePtr gi_node) {
  double val = 0;
  char* text = NULL;
  text = (char*)xmlGetProp(gi_node, (const xmlChar*)"decay_alpha");
  val = atof(text);
  free(text);
  xmlNodePtr child = gi_node->xmlChildrenNode;
  decision_att* atts = NULL;
  vector_2 curr = *zero_vec;
  vector_2 new = *zero_vec;
  vector_2 temp_vec = *zero_vec;
  gi* g = NULL;
  while(child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar*)"vector_2") == 0) {
      text = (char*)xmlGetProp(child, (const xmlChar*)"name");
      xml_read_vector_2(child, &temp_vec);
      if (strcmp(text, "curr_dir") == 0) {
	curr = temp_vec;
      }
      else if (strcmp(text, "curr_dir") == 0) {
	new = temp_vec;
      }
      free(text);
    }
    if (xmlStrcmp(child->name, (const xmlChar*)"decision_att") == 0) {
      atts = xml_read_attributes(child);
    }
    child = child->next;
  }
  g = create_gi();
  set_curr_dir(g, &curr);
  set_new_dir(g, &new);
  set_gi_attributes(g, atts);
  set_exp_decay_alpha(g, val);
  free_decision_att(atts);
  return g;
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
  xmlNodePtr child =  spawn_node->xmlChildrenNode;
  while(child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar*)"virt_pos") == 0) {
      xml_read_virt_pos(child, &spawn_pos);
    }
    child = child->next;
  }
  compound_spawner* spawn = create_compound_spawner(spawn_name, cap, spawn_pos.x, spawn_pos.y);
  free(spawn_name);
  return spawn;
}

void xml_write_compound(FILE* file_out, compound* comp) {
  fprintf(file_out, "<compound>\n");
  gen_node* cur = get_bodies(comp)->start;
  while(cur != NULL) {
    xml_write_body(file_out, (body*)cur->stored);
    cur = cur->next;
  }
  xml_write_gi(file_out, get_gi(comp));
  fprintf(file_out, "</compound>\n");
}

compound* xml_read_compound(xmlNodePtr comp_node) {
  compound* comp = create_compound();
  xmlNodePtr child = comp_node->xmlChildrenNode;
  body* cur_body = NULL;
  gi* g = NULL;
  while(child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar*)"body") == 0) {
      cur_body = xml_read_body(child);
      add_body_to_compound(comp, cur_body);
    }
    else if (xmlStrcmp(child->name, (const xmlChar*)"gi") == 0) {
      g = xml_read_gi(child);
    }
    child = child->next;
  }
  free_gi(get_gi(comp));
  set_gi(comp, g);
  return comp;
}

void xml_write_body(FILE* file_out, body* body) {
  fprintf(file_out, "<body poltergeist=\"%s\" >\n", get_polt_name(body->polt));
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
  fprintf(file_out, "</body>\n");
}

body* xml_read_body(xmlNodePtr body_node) {
  body* body = NULL;
  fizzle* fizz = NULL;
  collider* coll = NULL;
  polygon* poly = NULL;
  picture* pic = NULL;
  gen_list* temp_event_list = createGen_list();
  event* e = NULL;
  poltergeist* polt = NULL;
  char* text = (char*)xmlGetProp(body_node, (const xmlChar*)"poltergeist");
  if (strcmp(text, "NULL") != 0) {
    polt = make_poltergeist();
    set_polt_by_name(polt, text);
  }
  free(text);
  xmlNodePtr child = body_node->xmlChildrenNode;
  while(child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar*)"fizzle") == 0) {
      if (fizz != NULL) {
	fprintf(stderr, "xml body has > 1 fizzle children\n");
      }
      fizz = xml_read_fizzle(child);
    }
    else if (xmlStrcmp(child->name, (const xmlChar*)"polygon") == 0) {
      if (coll != NULL) {
	fprintf(stderr, "xml body has > 1 collider children\n");
      }
      poly = xml_read_polygon(child);
      coll = make_collider_from_polygon(poly);
    }
    else if (xmlStrcmp(child->name, (const xmlChar*)"picture") == 0) {
      if (pic != NULL) {
	fprintf(stderr, "xml body has > 1 picture children\n");
      }
      pic = xml_read_picture(child);
    }
    else if (xmlStrcmp(child->name, (const xmlChar*)"event") == 0) {
      e = xml_read_event(child);
      prependToGen_list(temp_event_list, createGen_node(e));
    }
    child = child->next;
  }
  body = createBody(fizz, coll);
  set_picture(body, pic);
  body->polt = polt;
  gen_node* curr = temp_event_list->start;
  while(curr != NULL) {
    e = (event*)curr->stored;
    add_event_to_body(body, e);
    curr = curr->next;
  }
  freeGen_list(temp_event_list);
  return body;
}

void xml_write_fizzle(FILE* file_out, fizzle* fizz) {
  fprintf(file_out, "<fizzle mass=\"%f\" rot_accel=\"%f\" rot_vel=\"%f\" bounce=\"%f\" >\n", fizz->mass, fizz->rot_acceleration, fizz->rot_velocity, get_bounce(fizz));
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
      text = xmlGetProp(child, (const xmlChar*)"name");
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
  fprintf(file_out, "<polygon sides=\"%d\" scale=\"%f\" rotation=\"%f\">\n", poly->sides, poly->scale, poly->rotation);
  xml_write_virt_pos(file_out, poly->center, "center");
  int buff_size = 16;
  char buff[buff_size];
  for (int i = 0; i < poly->sides; i++) {
    snprintf(buff, buff_size, "%d", i);
    xml_write_virt_pos(file_out, &(poly->base_corners[i]), buff);
  }
  fprintf(file_out, "</polygon>\n");
}

polygon* xml_read_polygon(xmlNodePtr polygon_node) {
  polygon* poly = NULL;
  xmlChar* text = NULL;
  double val = 0;
  int ival = 0;

  ival = get_int_prop(polygon_node, "sides");
  poly = createPolygon(ival);
  
  double scale = get_double_prop(polygon_node, "scale");

  val = get_double_prop(polygon_node, "rotation");
  poly->rotation = val;
  
  xmlNodePtr child = polygon_node->xmlChildrenNode;
  virt_pos vp;
  while(child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar*)"virt_pos") == 0) {
      xml_read_virt_pos(child, &vp);
      text = xmlGetProp(child, (const xmlChar*)"name");
      if (xmlStrcmp(text, (const xmlChar*)"center") == 0) {
	*(poly->center) = vp;
      }
      else {
	//should just be corner number
	ival = atoi((const char*)text);
	if (val >= poly->sides) {
	  fprintf(stderr, "xml file contained a corner indexed beyond polygons sides\n");
	}
	poly->base_corners[ival] = vp;
      }
      free(text);
    }
    child = child->next;
  }
  generate_normals_for_polygon(poly);
  set_scale(poly, scale);
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

						     

void xml_write_vector_2(FILE* file_out, vector_2* vec, char* name) {
  fprintf(file_out, "<vector_2 name=\"%s\" v1=\"%f\" v2=\"%f\" />\n", name, vec->v1, vec->v2);
}

void xml_read_vector_2(xmlNodePtr vec_node, vector_2* result) {
  double val = 0;

  val = get_double_prop(vec_node, "v1");
  result->v1 = val;

  val = get_double_prop(vec_node, "v2");
  result->v2 = val;
}

void xml_write_virt_pos(FILE* file_out, virt_pos* vp, char* name) {
  fprintf(file_out, "<virt_pos name=\"%s\" x=\"%d\" y=\"%d\" />\n", name, vp->x, vp->y);
}

void xml_read_virt_pos(xmlNodePtr vp_node, virt_pos* result) {
  int ival = 0;

  ival = get_int_prop(vp_node, "x");
  result->x = ival;

  ival = get_int_prop(vp_node, "y");
  result->y = ival;
}

void xml_write_attributes(FILE* file_out, decision_att* atts) {
  char* text = atts_to_text(atts);
  fprintf(file_out, "<decision_att bits=\"%s\" />\n", text);
  free(text);
}

decision_att* xml_read_attributes(xmlNodePtr atts_node) {
  char* text = (char*)xmlGetProp(atts_node, (const xmlChar*)"bits");
  decision_att* atts = text_to_atts(text);
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
