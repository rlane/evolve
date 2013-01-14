#include "critter.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static void critter_brain_step(float outputs[CRITTER_OUTPUT_SIZE], const float inputs[CRITTER_INPUT_SIZE], const struct critter_brain *brain);
static void critter_fetch_inputs(struct critter *critter, float inputs[CRITTER_INPUT_SIZE]);
static void critter_store_outputs(struct critter *critter, float outputs[CRITTER_OUTPUT_SIZE]);

/* Uniformly distributed between -1.0 and 1.0 */
static float
prng(void)
{
    return (random() / (RAND_MAX/2.0f)) - 1.0f;
}

/* Restrict to range [-1.0, 1.0] */
static float
clamp(float x)
{
    if (x > 1.0f) return 1.0f;
    if (x < -1.0f) return -1.0f;
    return x;
}

struct critter *
critter_create(void)
{
    struct critter *critter = calloc(1, sizeof(*critter));
    assert(critter);

    int i, j;

    for (i = 0; i < CRITTER_OUTPUT_SIZE; i++) {
	for (j = 0; j < CRITTER_INPUT_SIZE; j++) {
	    critter->brain.weights[i][j] = prng();
	}
    }

    for (i = 0; i < CRITTER_MEMORY_SIZE; i++) {
	critter->brain.weights[CO_MEMORY_START+i][CI_MEMORY_START+i] = 1.0;
    }

    for (i = 0; i < CRITTER_MEMORY_SIZE; i++) {
	critter->brain.memory[i] = prng();
    }

    critter->x = prng() * 10.0f;
    critter->y = prng() * 10.0f;
    critter->heading = prng() * M_PI;

    return critter;
}

void
critter_destroy(struct critter *critter)
{
    free(critter);
}

void
critter_dump(struct critter *critter, FILE *file)
{
#define FMT "%+.2f"

    int i, j;

    fprintf(file, "Position: "FMT", "FMT"\n", critter->x, critter->y);
    fprintf(file, "Velocity: "FMT", "FMT"\n", critter->vx, critter->vy);
    fprintf(file, "Heading: "FMT"\n", critter->heading);

    fprintf(file, "Brain matrix:\n");
    for (i = 0; i < CRITTER_OUTPUT_SIZE; i++) {
	for (j = 0; j < CRITTER_INPUT_SIZE; j++) {
	    fprintf(file, "%+.2f ", critter->brain.weights[i][j]);
	}
	fprintf(file, "\n");
    }

    fprintf(file, "Memory:\n");
    for (i = 0; i < CRITTER_MEMORY_SIZE; i++) {
	fprintf(file, "%+.2f ", critter->brain.memory[i]);
    }
    fprintf(file, "\n");

    fprintf(file, "Output accel: "FMT"\n", critter->out_accel);
    fprintf(file, "Output turn: "FMT"\n", critter->out_turn);

#undef FMT
}

void
critter_think(struct critter *critter)
{
    float inputs[CRITTER_INPUT_SIZE];
    float outputs[CRITTER_OUTPUT_SIZE];

    critter_fetch_inputs(critter, inputs);
    critter_brain_step(outputs, inputs, &critter->brain);
    critter_store_outputs(critter, outputs);
}

static void
critter_brain_step(float outputs[CRITTER_OUTPUT_SIZE],
	           const float inputs[CRITTER_INPUT_SIZE],
		   const struct critter_brain *brain)
{
    int i, j;
    for (i = 0; i < CRITTER_OUTPUT_SIZE; i++) {
	float sum = 0.0;
	for (j = 0; j < CRITTER_INPUT_SIZE; j++) {
	    sum += inputs[j] * brain->weights[i][j];
	}
	outputs[i] = clamp(sum);
    }
}

#if 0
/* Keep GCC from optimizing critter_brain_step away. */
void
critter_brain_step_wrapper(float outputs[CRITTER_OUTPUT_SIZE],
	                   const float inputs[CRITTER_INPUT_SIZE],
		           const struct critter_brain *brain)
{
    critter_brain_step(outputs, inputs, brain);
}
#endif

static void
critter_fetch_inputs(struct critter *critter, float inputs[CRITTER_INPUT_SIZE])
{
    inputs[CI_SPEED] = 0.0;
    inputs[CI_LIGHT_ANGLE] = 0.0;
    inputs[CI_ONE] = 1.0;
    memcpy(inputs+CI_MEMORY_START, critter->brain.memory, sizeof(critter->brain.memory));
}

static void
critter_store_outputs(struct critter *critter, float outputs[CRITTER_OUTPUT_SIZE])
{
    critter->out_accel = outputs[CO_ACCEL];
    critter->out_turn = outputs[CO_TURN];

    /* Memory changes slowly */
    int i;
    for (i = 0; i < CRITTER_MEMORY_SIZE; i++) {
	float *x = &critter->brain.memory[i];
	float y = outputs[CO_MEMORY_START+i];
	*x = (*x * 7.0f + y) / 8.0f;
    }
}

void
critter_act(struct critter *critter)
{
    const float tick_length = 0.016;
    const float world_size = 100.0f;
    const float bounciness = 0.8f;

    critter->x += critter->vx*tick_length*0.5;
    critter->y += critter->vy*tick_length*0.5;
    critter->vx += critter->out_accel*cosf(critter->heading)*tick_length;
    critter->vy += critter->out_accel*sinf(critter->heading)*tick_length;
    critter->x += critter->vx*tick_length*0.5;
    critter->y += critter->vy*tick_length*0.5;

    if (critter->x > world_size) {
        critter->x = world_size;
        critter->vx *= -bounciness;
    }

    if (critter->x < -world_size) {
        critter->x = -world_size;
        critter->vx *= -bounciness;
    }

    if (critter->y > world_size) {
        critter->y = world_size;
        critter->vy *= -bounciness;
    }

    if (critter->y < -world_size) {
        critter->y = -world_size;
        critter->vy *= -bounciness;
    }
}
