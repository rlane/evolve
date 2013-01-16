#ifndef EVOLVE_H
#define EVOLVE_H

struct critter;

#define MAX_CRITTERS 16

extern int screen_width;
extern int screen_height;
extern struct critter *critters[MAX_CRITTERS];
extern float light_x, light_y;

#endif
