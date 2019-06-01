#ifndef FILE_MAP_IO_SEEN
#define FILE_MAP_IO_SEEN
#include <libxml/parser.h>
#include "map.h"

map* load_map(char* filename);

void xml_write_map(FILE* file_out, map* map);
map* xml_read_map(xmlNodePtr map_node);

void test_xml_parse();
void test_plane_parse(plane* plane);

void xml_write_load_zone(FILE* file_out, load_zone* lz);
load_zone* xml_read_load_zone(xmlNodePtr lz_node);

void xml_write_plane(FILE* file_out, plane* plane);
plane* xml_read_plane(xmlNodePtr plane_node);

void xml_write_event(FILE* file_out, event* e);
event* xml_read_event(xmlNodePtr event_node);

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
		    


#endif 
