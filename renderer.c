#include <math.h>
#include <GL/glew.h>
#include "evolve.h"
#include "critter.h"

void
renderer_init(void)
{
    glViewport(0, 0, screen_width, screen_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double size = 100.0;
    double aspect = (double)screen_width/screen_height;
    gluOrtho2D(-size, size, -size/aspect, size/aspect);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.1, 0.1, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_MULTISAMPLE);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void
renderer_draw(void)
{
    /* Fade out the screen for a trail effect */
    {
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glColor4f(0.1f, 0.1f, 0.2f, 0.25);
        glBegin(GL_QUADS);
        glVertex2f(-1.0f, -1.0f);
        glVertex2f(1.0f, -1.0f);
        glVertex2f(1.0f, 1.0f);
        glVertex2f(-1.0f, 1.0f);
        glEnd();
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    /* Draw a circle for the light source */
    {
        glLoadIdentity();
        glTranslatef(10.0f, 10.0f, 0.0f);
        glColor4f(0.91f, 0.93f, 0.10f, 1.0f);
        glBegin(GL_LINE_LOOP);
        const float radius = 3.0f;
        const int n = 32;
        int i;
        for (i = 0; i < n; i++) {
            float a = i*M_PI*2.0f/n;
            glVertex2f(cos(a)*radius,sin(a)*radius);
        }
        glEnd();
    }


    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    int i;
    for (i = 0; i < MAX_CRITTERS; i++) {
        struct critter *critter = critters[i];
        if (!critter) {
            continue;
        }

        glLoadIdentity();

        glTranslatef(critter->x, critter->y, 0.0f);
        glRotatef(critter->heading*180.0f/M_PI, 0.0f, 0.0f, 1.0f);

        glBegin(GL_TRIANGLES);
        glVertex3f(4.0f, 0.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, 0.0f);
        glEnd();
    }
}
