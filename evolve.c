#include "evolve.h"
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
#include "renderer.h"

static void set_video_mode(void);
static void handle_events(void);
static void handle_key_down(SDL_keysym *keysym);

SDL_Surface *screen;
int screen_width = 800;
int screen_height = 600;
struct critter *critters[MAX_CRITTERS];

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    signal(SIGINT, SIG_DFL);

    set_video_mode();

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "glewInit failed\n");
        return 1;
    }

    int i;
    for (i = 0; i < MAX_CRITTERS; i++) {
        critters[i] = critter_create();
    }

    renderer_init();

    int tick = 0;
    while (1) {
        handle_events();

        int i, j;
        for (i = 0; i < 10; i++) {
            for (j = 0; j < MAX_CRITTERS; j++) {
                struct critter *critter = critters[j];
                if (critter) critter_think(critter);
            }
            for (j = 0; j < MAX_CRITTERS; j++) {
                struct critter *critter = critters[j];
                if (critter) critter_act(critter);
            }
        }

        renderer_draw();
        SDL_GL_SwapBuffers();
        SDL_Flip(screen);

        usleep(16*1000);
        tick++;
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
    default:
        break;
    }
}
