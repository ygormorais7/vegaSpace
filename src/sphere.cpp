#include "../includes/sphere.h"
#include <cmath>

void sphere_init(Esfera& e, float speed) {
    e.active   = false;
    e.speed = speed;
}

void sphere_fire(Esfera& e, const Ship& ship, float d) {
    if (e.active) return;
    e.active = true;
    float ry = ship.roty * M_PI/180.0f;
    float rx = ship.rotx * M_PI/180.0f;
    e.dirx = -sinf(ry);
    e.diry =  sinf(rx)*cosf(ry);
    e.dirz = -cosf(ry)*cosf(rx);
    e.x = ship.posx + d*e.dirx;
    e.y = ship.posy + d*e.diry;
    e.z = ship.posz + d*e.dirz;
}

void sphere_update(Esfera& e, float limit) {
    if (!e.active) return;
    e.x += e.dirx * e.speed;
    e.y += e.diry * e.speed;
    e.z += e.dirz * e.speed;
    if (fabsf(e.x) > limit || fabsf(e.y) > limit || fabsf(e.z) > limit)
        e.active = false;
}

void sphere_draw(const Esfera& e) {
    if (!e.active) return;
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    GLfloat amb[]  = {0.0f,0.1f,0.3f,1.0f},
            dif[]  = {0.0f,0.3f,1.0f,1.0f},
            spec[] = {0.4f,0.6f,1.0f,1.0f};
    glMaterialfv(GL_FRONT,GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT,GL_DIFFUSE, dif);
    glMaterialfv(GL_FRONT,GL_SPECULAR,spec);
    glMaterialf (GL_FRONT,GL_SHININESS,64.0f);
    glPushMatrix();
      glTranslatef(e.x, e.y, e.z);
      glutSolidSphere(0.02,20,16);
    glPopMatrix();
    glPopAttrib();
}