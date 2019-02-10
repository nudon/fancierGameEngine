/*
came up with a design plan for tethering objects together, mostly involves adding a special thing to fizzle structs which contains a pointer to 2 points, some distance of the tether, and probably some int/double representing strength of tether. only apply fore if distance between points is greater than tether

mostly thinking about some tangental issues resulting from segmenting organisms into multiple bodies, such as how the poltergeist works out and all that.

Thinking that each body part should have the characteristic byte-pattern thing so thing can have varied attributes accross body-parts. Might also introduce 2 byte-patterens, 1 that describes the entire organism and 1 thats just the body part. global things like aggression can be attatched to organism, other things like weak or invulnerable can be attached to body parts.

Would probably have to re-write poltergeist code either way. 1 way would be having a organism poltergeist which will do some stuff and manipulate limbs or do something. other would have poltergeists for each body but I can:t think of that being practical.



Also, would re-write the global loops to take the single-organism things, which iteslf would contain a linked list of bodies inside organism. 
 */

//first, teather structures
//had idea, define scales on points
//so, if there's > d displacement, scales determine how much objects move toward eachother
//1 issue, defined like this I need a reference to both object's fizzles
//I'd also need to store some list//array of tethers in fizzles
//but then evaluating tether forces would be annoying

//simpler way, define tethers as 1 way, anchor and free-point
//only apply movement to free-point
//if I want a 2-way tether, create 2 1-way tethers
//restricts how I can tie things together, but simpler

//multiple tethers, 2 ways of evaluating
//either iterate over fizzle objects and keep track of things
//or, have a seperate list of all tethers, iterate and apply things there
//would iterate over all tethers, adding resulting forces in fizzle fields
//when updating net of fizzle, apply tether then zero out tether field,
//very similar to how I reset impact
#include "compound.h"


