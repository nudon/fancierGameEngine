#include <stdio.h>
#include <stdlib.h>
#include "map_io.h"

//was thinking of including a bunch of files and havine various routines for converting to/from text

//some minor issues of list orders being reversed,
//can be fixed by starting at ends of lists and going forward

//updates
//need to include planes events and tethers


void test_xml_parse() {
  xmlDocPtr doc;
  xmlNodePtr cur;
  char* fn = "test.xml";
  doc = xmlParseFile(fn);

  if (doc != NULL) {
    cur = xmlDocGetRootElement(doc);
    if (cur != NULL) {
      printf("yay %s\n", cur->name);
    }
    else {
      
    }
  }
  else {
    
  }
}

void test_plane_parse(plane* test) {
  fprintf(stderr, "do I pass\n");
  char* fn1 = "test1.xml";
  char* fn2 = "test2.xml";
  FILE* file1 = fopen(fn1, "w+");
  FILE* file2 = fopen(fn2, "w+");

  char* xml_open = "<?xml version=\"1.0\"?>\n";


  fprintf(file1, xml_open);
  xml_write_plane(file1, test);
  fclose(file1);
  
  xmlDocPtr doc;
  xmlNodePtr cur;
  doc = xmlParseFile(fn1);
  cur = xmlDocGetRootElement(doc);
  //cur = cur->xmlChildrenNode;

  while (cur != NULL && xmlStrcmp(cur->name, (const xmlChar*)"plane") != 0) {
    cur = cur->next;
    printf("%s\n", cur->name);
  }

  plane* other_test = xml_read_plane(cur);
  
  fprintf(file2, xml_open);
  xml_write_plane(file2, other_test);
 
  fclose(file2);
  fprintf(stderr, "I guess go check %s and %s to see if they look similar\n", fn1, fn2);
}

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
  fprintf(file_out, "</plane>");
}

plane* xml_read_plane(xmlNodePtr plane_node) {
  xmlNodePtr child = plane_node->xmlChildrenNode;
  int w = -1,h = -1,r = -1,c = -1, z = -1;
  char* text = NULL;
  text = (char*)xmlGetProp(plane_node, (const xmlChar*)"rows");
  r = atoi(text);
  free(text);
  text = (char*)xmlGetProp(plane_node, (const xmlChar*)"cols");
  c = atoi(text);
  free(text);
  text = (char*)xmlGetProp(plane_node, (const xmlChar*)"cellW");
  w = atoi(text);
  free(text);
  text = (char*)xmlGetProp(plane_node, (const xmlChar*)"cellH");
  h = atoi(text);
  free(text);
  text = (char*)xmlGetProp(plane_node, (const xmlChar*)"zLevel");
  z = atof(text);
  free(text);
  text = (char*)xmlGetProp(plane_node, (const xmlChar*)"name");
  spatial_hash_map* map = create_shm(w,h,c,r);
  plane* plane = create_plane(map, text);
  free(text);
  set_z_level(plane, z);
  compound* comp = NULL;
  load_zone* lz = NULL;
  while (child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar*)"compound") == 0) {
      comp = xml_read_compound(child);
      add_compound_to_plane(plane, comp);
    }
    else if (xmlStrcmp(child->name, (const xmlChar*)"load_zone") == 0) {
      lz = xml_read_load_zone(child);
      add_load_zone_to_plane(plane, lz);
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


//almost done with these
//but realized that events have some shared data which is hard to put into xml
//thinking of rewriting it and having some specialized way to indicate shared data things
//like a standard polt taking in a body, and it creates an event and sets associated body
//
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

void xml_write_compound(FILE* file_out, compound* comp) {
  fprintf(file_out, "<compound>\n");
  gen_node* cur = get_bodies(comp)->start;
  xml_write_attributes(file_out, get_attributes(comp));
  while(cur != NULL) {
    xml_write_body(file_out, (body*)cur->stored);
    cur = cur->next;
  }
  fprintf(file_out, "</compound>\n");
}

compound* xml_read_compound(xmlNodePtr comp_node) {
  compound* comp = create_compound();
  xmlNodePtr body_node = comp_node->xmlChildrenNode;
  body* cur_body = NULL;
  decision_att* atts;
  while(body_node != NULL) {
    if (xmlStrcmp(body_node->name, (const xmlChar*)"body") == 0) {
      cur_body = xml_read_body(body_node);
      add_body_to_compound(comp, cur_body);
    }
    else if (xmlStrcmp(body_node->name, (const xmlChar*)"decision_att") == 0) {
      atts = xml_read_attributes(body_node);
      set_attributes(comp, atts);
      free_decision_att(atts);
    }
    body_node = body_node->next;
  }
  return comp;
}

void xml_write_body(FILE* file_out, body* body) {
  fprintf(file_out, "<body poltergeist=\"%s\" >\n", get_polt_name(body->polt));
  xml_write_fizzle(file_out, get_fizzle(body));
  xml_write_polygon(file_out, get_polygon(get_collider(body)));
  xml_write_picture(file_out, get_picture(body));
  fprintf(file_out, "</body>\n");
}

body* xml_read_body(xmlNodePtr body_node) {
  body* body = NULL;
  fizzle* fizz = NULL;
  collider* coll = NULL;
  polygon* poly = NULL;
  picture* pic = NULL;
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
    child = child->next;
  }
  body = createBody(fizz, coll);
  set_picture(body, pic);
  body->polt = polt;
  return body;
}

void xml_write_fizzle(FILE* file_out, fizzle* fizz) {
  fprintf(file_out, "<fizzle mass=\"%f\" rot_accel=\"%f\" rot_vel=\"%f\" >\n", fizz->mass, fizz->rot_acceleration, fizz->rot_velocity);
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
  double mass = -1, rot_vel = -1, rot_accel = -1;
  xmlChar* text = NULL;
  vector_2 vec;
  text = xmlGetProp(fizzle_node, (const xmlChar*)"mass");
  mass = atof((const char*)text);
  free(text);
  text = xmlGetProp(fizzle_node, (const xmlChar*)"rot_accel");
  rot_vel = atof((const char*)text);
  free(text);
  text = xmlGetProp(fizzle_node, (const xmlChar*)"rot_vel");
  rot_accel = atof((const char*)text);
  free(text);
  fizz->mass = mass;
  fizz->rot_acceleration = rot_accel;
  fizz->rot_velocity = rot_vel;
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
  text = xmlGetProp(polygon_node, (const xmlChar*)"sides");
  val = atoi((const char*)text);
  free(text);
  poly = createPolygon(val);
  text = xmlGetProp(polygon_node, (const xmlChar*)"scale");
  val = atof((const char*)text);
  free(text);
  poly->scale = val;
  text = xmlGetProp(polygon_node, (const xmlChar*)"rotation");
  val = atof((const char*)text);
  free(text);
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
  return poly;
}

void xml_write_picture(FILE* file_out, picture* pic) {
  fprintf(file_out, "<picture file_name=\"%s\" />\n", pic->file_name);
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
  xmlChar* text = NULL;
  double val = 0;
  text = xmlGetProp(vec_node, (const xmlChar*)"v1");
  val = atof((const char*)text);
  free(text);
  result->v1 = val;
  text = xmlGetProp(vec_node, (const xmlChar*)"v2");
  val = atof((const char*)text);
  free(text);
  result->v2 = val;
}

void xml_write_virt_pos(FILE* file_out, virt_pos* vp, char* name) {
  fprintf(file_out, "<virt_pos name=\"%s\" x=\"%d\" y=\"%d\" />\n", name, vp->x, vp->y);
}

void xml_read_virt_pos(xmlNodePtr vp_node, virt_pos* result) {
  xmlChar* text = NULL;
  double val = 0;
  text = xmlGetProp(vp_node, (const xmlChar*)"x");
  val = atoi((const char*)text);
  free(text);
  result->x = val;
  text = xmlGetProp(vp_node, (const xmlChar*)"y");
  val = atoi((const char*)text);
  free(text);
  result->y = val;
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
