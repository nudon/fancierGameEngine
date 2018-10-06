
#include "body.h"
//basic idea would be to calculate rebounding velocities based on conversation of momentum
//for simplicity, start with redirecting velocity by flipping it across the normal force and negating components
//would just take velocity, project onto normal, sutract result from original, get orthog component
//take parrellel component, negate it, than add orthog back, should be rebound velocity
//issue was grabbing normals. would just do a standard-dpi funciton call/copy
//though this time, grab information about which side/normal resulted in a collision. potentiall have to iterate over all points to grab all colliding normals, and just take an average normal or something

//will actually have modify some previous code to get colliding polgons and their colliding points
//will probably be a similar manner to other things, calling a function to find number of collisions, creating structur onto stack,then repeating traversal to fill structure
//could also malloc though, if it saves time, but I don't think it does

//so need traversals for finding colliding polygons

//and also traversals for finding colliding points

//variants for both for counting collisions and inserting results into some structure

//okay I'm listening to the mogs and about to drink
//lets do this shit
//also before I forget probably need to somehow manage problem of two onjects moving toward eachother, colliding, and how to solve collisions without repeating things, because naively I'll resolve the same collision twice
//kind of a hard topic, since you'd need to keep track of which object-pairs have resolved collisions
//actually thinking of doing it that way, just maintaining some pair-data structure of resolved collisions, do a search when resolving a new pair
//probably some unsorted array or linked list
//since I'm lazy, 
void resolve_collisions(spatial_hash_map* map, body* main_body) {
  //somehow need to connect fizzles with colliders
  fizzle* main_fizz = main_body->fizz;;
  collider* main_coll = main_body->coll;
  vector* occupied = main_coll->collider_node->active_cells; //may be old_cells, tbd
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
  //sets a new velocity vector, also need to figure out where to reset some force components to zero
  //yeah, force not being modified is odd. I guess if my dampening function works it will eventually zero-out, put kind of wasteful
  //
  //also, might be interesting to add some sort of snap to side feature
  //right now because of the high acceleration, when object is stopped, stops far away from object
  
  polygon* p1 = body1->coll->shape;
  polygon* p2 = body2->coll->shape;
  vector_2 normal_of_collision;
  //these simulate elasticity conditions
  //1 for fully elastic
  //0 for dead-stop
  double f1scale = 0;
  double f2scale = 0;
  get_normal_of_collision(body1, body2, &normal_of_collision);

  //cases this needs to cover
  //case of a large object hitting a small one, should cause the small one to move with it, with little change in own direction
  //opposite case, small object should rebound off of large object somehonew
  //need to find some general principle that determines how a vector can rebound like that
 
  //in my elucidite wisdom my notes on impulse are a 6th of a page,
  //think I figured shit out thoug
  //given two object, depening on things(mass or momentum?), can determine resulting directions of motion
  //for object, colliding with another object on its  side with normal vector, decompose vector into parallell and orthogonal components
  //beleive orhogonal component stays the same, parallell component is variable though.
  //thnk it gets scaled by (mass1 - mass2), or some such. if mass1 >>>> mass2, things hardly change. if equal, cancles motion for obj 1, if mass1 <<<< mass2, object1 rebounds reflecting off of normal

  vector_2 f1p, f1o, f2p, f2o, f1f, f2f;
  fizzle* fizz1 = body1->fizz, *fizz2 = body2->fizz;
  decompose_vector(&(fizz1->velocity), &normal_of_collision, &f1p, &f1o);
  decompose_vector(&(fizz2->velocity), &normal_of_collision, &f2p, &f2o);
  double diff = fizz1->mass - fizz2->mass;
  vector_2_scale(&f1p, diff, &f1f);
  vector_2_add(&f1f, &f1o, &f1f);
  vector_2_scale(&f1f, f1scale, &f1f);
  diff *= -1;
  vector_2_scale(&f2p, diff, &f2f);
  vector_2_add(&f2f, &f2o, &f2f);
  vector_2_scale(&f2f, f2scale, &f2f);

  fizz1->velocity = f1f;
  fizz2->velocity = f2f;
  //that should be it. somehow need to imporperate elascticitys things
  //probably just some modifier to diff
  //would be between 0 and 1, probably just take max of both fizz's elascticiy'
  //since a bouncy ball hitting a wall is still mostly elasctic((?)?)
  //all that should be left is maintaining the collision list and updating fizzles velocities
  //for fizzle list, thinking of maintaining some standard so that larger/smaller pointer is first, though it hardlny matters
  //otherwise collisions may be resolved multiple times 
}

void get_normal_of_collision(body* body1, body* body2, vector_2* result) {
  //just doing the vagually correct thing
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




/*
typedef struct {
  fizzle* fizz;
  collider* coll;
} body;
*/


body* createBody(fizzle* fizz, struct collider_struct* coll) {
  body* new = malloc(sizeof(body));
  new->fizz = fizz;
  new->coll = coll;
  coll->body = new;
  return new;
}

void freeBody(body* rm) {
  freeFizzle(rm->fizz);
  freeCollider(rm->coll);
  free(rm);
}

//also, kind of annoying, but I'll probably need to add a body pointer to the collider/list node somewhere
//because otherwise fuck dogs are barking
//otherwise when I'm going over collisions I have no way of knowing about other objects physical properties
//yeah, will add it to collider structure, can initialize it to null in createCollider, then fill it when creatingBody
