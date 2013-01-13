#include <GL/glew.h>

extern struct critter *critter;

void
renderer_init(void)
{
    glClearColor(0.1, 0.1, 0.3, 1.0);
}

void
renderer_draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
