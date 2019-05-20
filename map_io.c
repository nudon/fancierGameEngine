#include <stdio.h>
#include <stdlib.h>
#include "map_io.h"
//was thinking of including a bunch of files and havine various routines for converting to/from text

//asm-xml really only provides a parser, but outputting text should be easy
//*nervously pretending it's not going to be annoying*


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
}

void xml_write_plane(FILE* file_out, plane* plane) {
  box mat_dim = plane->map->matrix_dim;
  box cell_dim = plane->map->cell_dim;
  fprintf(file_out, "<plane rows=\"%d\" cols=\"%d\" cellH=\"%d\" cellW=\"%d\">\n", mat_dim.height, mat_dim.width, cell_dim.height, cell_dim.width );
  gen_node* cur_comp = plane->compounds_in_plane->start;
  while (cur_comp != NULL) {
    xml_write_compound(file_out, (compound*)cur_comp->stored);
    cur_comp = cur_comp->next;
  }
  fprintf(file_out, "</plane>");
}

plane* xml_read_plane(xmlNodePtr plane_node) {
  xmlNodePtr child = plane_node->xmlChildrenNode;
  int w = -1,h = -1,r = -1,c = -1;
  char* text = NULL;
  double val = 0;
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
  spatial_hash_map* map = create_shm(w,h,c,r);
  plane* plane = create_plane(map);
  compound* comp = NULL;
  while (child != NULL) {
    if (xmlStrcmp(child->name, (const xmlChar*)"compound") == 0) {
      comp = xml_read_compound(child);
      add_compound_to_plane(plane, comp);
    }
    child = child->next;
  }
  return plane;
}


void xml_write_compound(FILE* file_out, compound* comp) {
  fprintf(file_out, "<compound>\n");
  gen_node* cur = get_bodies(comp)->start;
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
  while(body_node != NULL) {
    if (xmlStrcmp(body_node->name, (const xmlChar*)"body") == 0) {
      cur_body = xml_read_body(body_node);
      add_body_to_compound(comp, cur_body);
    }
    body_node = body_node->next;
  }
  return comp;
}

void xml_write_body(FILE* file_out, body* body) {
  fprintf(file_out, "<body>\n");
  xml_write_fizzle(file_out, get_fizzle(body));
  xml_write_polygon(file_out, get_polygon(get_collider(body)));
  fprintf(file_out, "</body>\n");
}

body* xml_read_body(xmlNodePtr body_node) {
  body* body = NULL;
  fizzle* fizz = NULL;
  collider* coll = NULL;
  polygon* poly = NULL;
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
    child = child->next;
  }
  body = createBody(fizz, coll);
  return body;
}

void xml_write_fizzle(FILE* file_out, fizzle* fizz) {
  fprintf(file_out, "<fizzle mass=\"%f\" >\n", fizz->mass);
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
  double val;
  xmlChar* text = NULL;
  vector_2 vec;
  text = xmlGetProp(fizzle_node, (const xmlChar*)"mass");
  val = atof((const char*)text);
  free(text);
  fizz->mass = val;
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
  return poly;
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

						    
		    
