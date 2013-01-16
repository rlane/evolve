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

static float
clamp_angle(float a)
{
    if (a > M_PI) a -= 2*M_PI;
    if (a < -M_PI) a += 2*M_PI;
    return a;
}

static void
critter_init_common(struct critter *critter)
{
    int i;
    for (i = 0; i < CRITTER_MEMORY_SIZE; i++) {
	critter->brain.memory[i] = prng();
    }

    critter->x = prng() * 10.0f;
    critter->y = prng() * 10.0f;
    critter->heading = prng() * M_PI;
}

struct critter *
critter_create_random(void)
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

    critter_init_common(critter);

    return critter;
}

struct critter *
critter_create_child(const struct critter *parent1, const struct critter *parent2)
{
    struct critter *critter = calloc(1, sizeof(*critter));
    assert(critter);

    const struct critter *parents[2] = { parent1, parent2 };
    int num_parents = 2;
    int inherit_rows = random() % 2;

    int i, j;
    for (i = 0; i < CRITTER_OUTPUT_SIZE; i++) {
	for (j = 0; j < CRITTER_INPUT_SIZE; j++) {
            int idx = (inherit_rows ? i : j) % num_parents;
            const struct critter *parent = parents[idx];
	    critter->brain.weights[i][j] = parent->brain.weights[i][j];
        }
    }

    /* Mutate */
    i = random() % CRITTER_OUTPUT_SIZE;
    j = random() % CRITTER_INPUT_SIZE;
    critter->brain.weights[i][j] += prng()/10.0f;

    critter_init_common(critter);

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
    fprintf(file, "Speed: "FMT"\n", sqrtf(powf(critter->vx, 2) + powf(critter->vy, 2)));
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
    {
        float speed = sqrtf(critter->vx*critter->vx + critter->vy*critter->vy);
        inputs[CI_SPEED] = clamp(logf(fabsf(speed))/log(20.0f));
        if (speed < 0.0f) inputs[CI_SPEED] *= -1.0f;
        assert(inputs[CI_SPEED] <= 1.0f && inputs[CI_SPEED] >= -1.0f);
    }

    {
        float light_x = 10.0f;
        float light_y = 10.0f;
        float dx = light_x - critter->x;
        float dy = light_y - critter->y;
        float a = atan2f(dy, dx);
        float da = clamp_angle(a - critter->heading);
        inputs[CI_LIGHT_ANGLE] = da/M_PI;
        assert(inputs[CI_LIGHT_ANGLE] <= 1.0f && inputs[CI_LIGHT_ANGLE] >= -1.0f);
    }
    inputs[CI_ONE] = 1.0;
    memcpy(inputs+CI_MEMORY_START, critter->brain.memory, sizeof(critter->brain.memory));
}

static void
critter_store_outputs(struct critter *critter, float outputs[CRITTER_OUTPUT_SIZE])
{
    critter->out_accel = outputs[CO_ACCEL] * 10.0f;
    critter->out_turn = outputs[CO_TURN] * 1.0f;

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

    /* Compute drag */
    float drag_coefficient = 0.05f;
    float v_squared = critter->vx*critter->vx + critter->vy*critter->vy;
    if (v_squared > 0.001f) {
        float drag_acc = drag_coefficient*v_squared;
        float v_mag = sqrt(v_squared);
        float vx_norm = critter->vx/v_mag;
        float vy_norm = critter->vy/v_mag;
        critter->vx -= drag_acc*vx_norm*tick_length;
        critter->vy -= drag_acc*vy_norm*tick_length;
    }

    /* Rotation */
    critter->heading += critter->out_turn * tick_length;
    critter->heading = clamp_angle(critter->heading);
}
