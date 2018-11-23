
#include "body.h"


void resolve_collisions(spatial_hash_map* map, body* main_body) {

  fizzle* main_fizz = main_body->fizz;;
  collider* main_coll = main_body->coll;
  vector* occupied = main_coll->collider_node->active_cells;
  int size = number_of_unique_colliders_in_entries(map, occupied);
  body* curr;
  collider* collisions[size];
  unique_colliders_in_entries(map, occupied, collisions);
  for (int i = 0; i < size; i++) {
    curr = collisions[i]->body;
    if (curr != main_body) {
      resolve_collision(main_body, collisions[i]->body);
    }
  }
}

void resolve_collision(body* body1, body* body2) {
  //goal, add some forces to either body to simulate a collision

    //cases this needs to cover
  //case of a large object hitting a small one, should cause the small one to move with it, with little change in own direction
  //opposite case, small object should rebound off of large object somehonew
  //need to find some general principle that determines how a vector can rebound like that
 
  //in my elucidite wisdom my notes on impulse are a 6th of a page,
  //given two object, depening on things(mass or momentum?), can determine resulting directions of motion
  //for object, colliding with another object on its  side with normal vector, decompose vector into parallell and orthogonal components
  //beleive orhogonal component stays the same, parallell component is variable though.
  //thnk it gets scaled by (mass1 - mass2), or some such. if mass1 >>>> mass2, things hardly change. if equal, cancles motion for obj 1, if mass1 <<<< mass2, object1 rebounds reflecting off of normal

  //messed up a bit
  //if mass of objects are equal
  //then both objects should move with same velocity in end. depending on how many objects are moving
  //final velocity may be zero or anything else really.
  //also think I need to displace objects by mtv along with changing velocities
 

  
  polygon* p1 = body1->coll->shape;
  polygon* p2 = body2->coll->shape;
  vector_2 normal_of_collision;
  //these simulate elasticity conditions
  //1 for fully elastic
  //0 for dead-stop
  double f1scale = 1.1;
  double f2scale = 1.1;
  get_normal_of_collision(body1, body2, &normal_of_collision);
  int actual_collision = find_mtv_of_polygons(p1, p2, &normal_of_collision);
  //potentially theres' no catual collision since everything has been coarse grain at this point
  if (actual_collision != 0) {
    fprintf(stderr, "theres an actual collision, do things\n");
    vector_2 f1p, f1o, f2p, f2o, f1f, f2f;
    fizzle* fizz1 = body1->fizz, *fizz2 = body2->fizz;
    decompose_vector(&(fizz1->velocity), &normal_of_collision, &f1p, &f1o);
    decompose_vector(&(fizz2->velocity), &normal_of_collision, &f2p, &f2o);
    //originally
    //double diff = fizz1->mass - fizz2->mass
    double diff = fizz1->mass / (fizz2->mass + fizz1->mass);
    vector_2_scale(&f1p, diff, &f1f);
    vector_2_add(&f1f, &f1o, &f1f);
    vector_2_scale(&f1f, f1scale, &f1f);
    diff *= diff = -1 * fizz2->mass / (fizz2->mass + fizz1->mass);
    vector_2_scale(&f2p, diff, &f2f);
    vector_2_add(&f2f, &f2o, &f2f);
    vector_2_scale(&f2f, f2scale, &f2f);
    fprintf(stderr, "f1 final velocity:");
    print_vector(&f1f);
    fprintf(stderr, "f2 final velocity:");
    print_vector(&f2f);
    fizz1->velocity = f1f;
    fizz2->velocity = f2f;
  }
}

void get_normal_of_collision(body* body1, body* body2, vector_2* result) {
  //actually hard to obtain normal of collision using Seperation along axis
  //because projecting onto normal of some side may min/maxes of shapes to overlap
  //even though they may not intersect along that side.
  //just returning differences between normals for now
  virt_pos diff;
  virt_pos_sub(&(body1->coll->shape->center), &(body2->coll->shape->center), &diff);
  virt_pos_to_vector_2(&diff, result);
  return;
  //old idea, involved grabbing normals of vectors which result in overlaps when projecting polygons onto them
  //broken for squares, because both sides trigger this, to resulting sum is zero_vector
  /*
  vector_2 coll1_avg = *zero_vec, coll2_avg = *zero_vec, avg = *zero_vec;
  int count = 0;
  //need to do all below for potentially every normal of both polygons
  double offset_x, offset_y;
  double p1_min, p1_max, p2_min, p2_max, max, min;
  virt_pos relative_point;
  vector_2 normal_vector;
  int isDone = 0, ret = 1, index = 0, polygon = 1;
  polygon* poly = p1;;
  while(!isDone) {
    if (polygon == 1) {
      if (index < p1->sides) {
	get_actual_normal(p1, index, &normal_vector);
	index++;
      }
      else {
	polygon = 2;
	poly = p2;
	index = 0;
	if (count != 0) {
	  vector_2_scale(&avg, 1 / count, &coll1_avg);
	}
	avg = *zero_vec;
	count = 0;
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

    extreme_projections_of_polygon(p1, &p1->center, &normal_vector, &p1_min, &p1_max);
    extreme_projections_of_polygon(p2, &p1->center, &normal_vector, &p2_min, &p2_max);
     if ( p1_max < p2_min || p2_max < p1_min) {
     vector_2_add(&normal_vector, &avg, &avg);
       count++;
       
       }
       }
  if (count != 0) {
    vector_2_scale(&avg, 1 / count, &coll2_avg);
  }
  //got averages now, do the thing now
  */
}


body* createBody(fizzle* fizz, struct collider_struct* coll) {
  body* new = malloc(sizeof(body));
  new->fizz = fizz;
  new->coll = coll;
  new->polt = NULL;
  coll->body = new;
  
  return new;
}

void freeBody(body* rm) {
  freeFizzle(rm->fizz);
  freeCollider(rm->coll);
  free(rm);
}
