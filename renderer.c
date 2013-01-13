#include <GL/glew.h>
#include "critter.h"

extern struct critter *critter;
extern int screen_width, screen_height;

void
renderer_init(void)
{
    glViewport(0, 0, screen_width, screen_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double size = 100.0;
    double aspect = (double)screen_width/screen_height;
    gluOrtho2D(size, -size, -size/aspect, size/aspect);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.1, 0.1, 0.2, 1.0);

    glEnable(GL_MULTISAMPLE);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
}

void
renderer_draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glRotatef(critter->heading, 0.0f, 0.0f, 1.0f);
    glTranslatef(critter->x, critter->y, 0.0f);

    glBegin(GL_TRIANGLES);
    glVertex3f( 0.0f, 1.5f, 0.0f);
    glVertex3f(-1.0f,-1.0f, 0.0f);
    glVertex3f( 1.0f,-1.0f, 0.0f);
    glEnd();
}
