#include "asteroid.h"
#include <cstdio>
#include <cstdlib>

void asteroid_init(Asteroid& a, const char* model_path,
                   float px, float py, float pz,
                   float rx, float ry, float rz)
{
    a.model = glmReadOBJ((char*)model_path, true);
    if (!a.model) {
        fprintf(stderr, "Erro: não pôde carregar %s\n", model_path);
        exit(1);
    }
    glmUnitize(a.model);
    glmFacetNormals(a.model);
    glmVertexNormals(a.model, 90.0);
    a.posx = px; a.posy = py; a.posz = pz;
    a.rotx = rx; a.roty = ry; a.rotz = rz;
}

void asteroid_draw(const Asteroid& a)
{
    if (!a.model) return;
    glPushMatrix();
      glTranslatef(a.posx, a.posy, a.posz);
      glRotatef(a.rotx, 1,0,0);
      glRotatef(a.roty, 0,1,0);
      glRotatef(a.rotz, 0,0,1);
      glmDraw(a.model, GLM_SMOOTH);
    glPopMatrix();
}

void asteroid_cleanup(Asteroid& a)
{
    if (a.model) {
        glmDelete(a.model);
        a.model = nullptr;
    }
}