#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <SDL.h>
#include <GL/glew.h>

#include "critter.h"

static void handle_events(void);
static void handle_key_down(SDL_keysym *keysym);
static void draw(void);

SDL_Surface *screen;
int screen_width = 800;
int screen_height = 600;
struct critter *critter;

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    signal(SIGINT, SIG_DFL);

    screen = SDL_SetVideoMode(screen_width, screen_height, 0, SDL_OPENGL);
    if (screen == NULL) {
        fprintf(stderr, "SDL_SetVideoMode failed\n");
        return 1;
    }

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "glewInit failed\n");
        return 1;
    }

    struct critter *critter = critter_create();

    fprintf(stderr, "Created critter:\n");
    critter_dump(critter, stderr);
    fprintf(stderr, "\n");

    int tick = 0;
    while (1) {
        handle_events();
        critter_think(critter);
        critter_act(critter);

        draw();
        SDL_GL_SwapBuffers();
        SDL_Flip(screen);

        if (tick % 100 == 0) {
            fprintf(stderr, "Current critter (tick=%d):\n", tick);
            critter_dump(critter, stderr);
            fprintf(stderr, "\n");
        }

        usleep(16*1000);
        tick++;
    }

    return 0;
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
    default:
        break;
    }
}

static void
draw(void)
{
    glClearColor(0.1, 0.1, 0.3, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
