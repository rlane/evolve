#include "evolve.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <SDL.h>
#include <GL/glew.h>

#include "critter.h"
#include "renderer.h"

#define EVOLVE_INTERVAL 640

static void set_video_mode(void);
static void handle_events(void);
static void handle_key_down(SDL_keysym *keysym);
static void evolve(void);

SDL_Surface *screen;
int screen_width = 800;
int screen_height = 600;
struct critter *critters[MAX_CRITTERS];
int tick;
int time_multiplier = 4;

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    signal(SIGINT, SIG_DFL);

    set_video_mode();

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "glewInit failed\n");
        return 1;
    }

    renderer_init();

    int i;
    for (i = 0; i < MAX_CRITTERS; i++) {
        critters[i] = critter_create_random();
    }

    while (1) {
        uint32_t frame_start_ms = SDL_GetTicks();

        handle_events();

        int i, j;
        for (i = 0; i < time_multiplier; i++) {
            tick++;
            for (j = 0; j < MAX_CRITTERS; j++) {
                struct critter *critter = critters[j];
                if (critter) critter_think(critter);
            }
            for (j = 0; j < MAX_CRITTERS; j++) {
                struct critter *critter = critters[j];
                if (critter) critter_act(critter);
            }
            if (tick % EVOLVE_INTERVAL == 0) {
                evolve();
            }
        }

        renderer_draw();
        SDL_GL_SwapBuffers();
        SDL_Flip(screen);

        uint32_t frame_end_ms = SDL_GetTicks();
        int sleep_ms = 16 - (frame_end_ms - frame_start_ms);
        if (sleep_ms > 0) {
            SDL_Delay(sleep_ms);
        }
    }

    return 0;
}

static void
set_video_mode(void)
{
    int flags = SDL_OPENGL | SDL_HWSURFACE | SDL_DOUBLEBUF;
    int bpp = 0;

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    if (!SDL_VideoModeOK(screen_width, screen_height, bpp, flags)) {
        fprintf(stderr, "Failed to enable multisampling.\n");
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
    }

    screen = SDL_SetVideoMode(screen_width, screen_height, bpp, flags);
    if (screen == NULL) {
        fprintf(stderr, "SDL_SetVideoMode failed\n");
        exit(1);
    }
}

static void handle_events(void)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_KEYDOWN:
            handle_key_down(&event.key.keysym);
            break;
        case SDL_QUIT:
            exit(0);
            break;
        }
    }
}

static void handle_key_down(SDL_keysym *keysym)
{
    switch (keysym->sym) {
    case SDLK_ESCAPE:
        exit(0);
        break;
    case SDLK_RIGHT:
        time_multiplier *= 2;
        fprintf(stderr, "time multiplier: %dx\n", time_multiplier);
        break;
    case SDLK_LEFT:
        if (time_multiplier > 1) {
            time_multiplier /= 2;
        }
        fprintf(stderr, "time multiplier: %dx\n", time_multiplier);
        break;
    default:
        break;
    }
}

/* Higher the result the worse the fitness */
static float
fitness(const struct critter *critter)
{
    float dx = critter->x - 10.0f;
    float dy = critter->y - 10.0f;
    return sqrtf(dx*dx + dy*dy);
}

static int
critter_sorter(const void *_a, const void *_b)
{
    const struct critter *a = *(const struct critter **)_a;
    const struct critter *b = *(const struct critter **)_b;
    if (a == NULL && b == NULL) {
        return 0;
    } else if (a == NULL) {
        return -1;
    } else if (b == NULL) {
        return 1;
    } else {
        return (int)(fitness(a) - fitness(b));
    }
}

static void
evolve(void)
{
    fprintf(stderr, "starting evolution step\n");

    qsort(critters, MAX_CRITTERS, sizeof(critters[0]), critter_sorter);

    int i;
    for (i = MAX_CRITTERS/2; i < MAX_CRITTERS; i++) {
        if (critters[i]) {
            fprintf(stderr, "culling loser critter with score %.2f\n", fitness(critters[i]));
            critter_destroy(critters[i]);
            critters[i] = NULL;
        }
    }

    for (i = MAX_CRITTERS/2; i < MAX_CRITTERS; i++) {
        const struct critter *parent1 = critters[random() % (MAX_CRITTERS/2)];
        const struct critter *parent2 = critters[random() % (MAX_CRITTERS/2)];
        assert(parent1 && parent2);
        assert(!critters[i]);
        critters[i] = critter_create_child(parent1, parent2);
    }

    /* Kill 1/8 of the winners too */
    for (i = 0; i < MAX_CRITTERS/8; i++) {
        int j = random() % (MAX_CRITTERS/2);
        if (critters[j]) {
            fprintf(stderr, "culling winner critter with score %.2f\n", fitness(critters[j]));
            critter_destroy(critters[j]);
            critters[j] = critter_create_random();
        }
    }

    fprintf(stderr, "finished evolution step\n");
}
