#ifndef CRITTER_H
#define CRITTER_H

#include <stdio.h>

enum critter_input {
    CI_SPEED,
    CI_LIGHT_ANGLE,
    CI_ONE,
    CI_MEMORY_START,
};

enum critter_output {
    CO_ACCEL,
    CO_TURN,
    CO_MEMORY_START,
};

#define CRITTER_MEMORY_SIZE 4
#define CRITTER_INPUT_SIZE (CI_MEMORY_START + CRITTER_MEMORY_SIZE)
#define CRITTER_OUTPUT_SIZE (CO_MEMORY_START + CRITTER_MEMORY_SIZE)

struct critter_brain {
    float memory[CRITTER_MEMORY_SIZE];
    float weights[CRITTER_OUTPUT_SIZE][CRITTER_INPUT_SIZE];
};

struct critter {
    struct critter_brain brain;

    /* Outputs from the brain */
    float out_accel;
    float out_turn;

    /* Physics state */
    float x, y;
    float vx, vy;
    float heading; /* radians */
};

struct critter *critter_create_random(void);
struct critter *critter_create_child(const struct critter *parent1, const struct critter *parent2);
void critter_destroy(struct critter *critter);
void critter_dump(struct critter *critter, FILE *file);
void critter_think(struct critter *critter);
void critter_act(struct critter *critter);

#endif
